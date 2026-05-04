import argparse
import json
import os
import subprocess
import sys
import time
from collections import Counter, defaultdict
from datetime import datetime
from itertools import product
from pathlib import Path


def _print_cmd(cmd_list):
    return " ".join(f'"{c}"' if (" " in c or "\t" in c) else c for c in cmd_list)


def run_checked(cmd_list, cwd=None, env=None, timeout_s=None, log_path=None, allow_nonzero=False):
    print(f"$ {_print_cmd([str(x) for x in cmd_list])}")
    sys.stdout.flush()

    stdout = None
    stderr = None
    log_f = None
    try:
        if log_path is not None:
            log_path.parent.mkdir(parents=True, exist_ok=True)
            log_f = open(log_path, "w", encoding="utf-8")
            stdout = log_f
            stderr = subprocess.STDOUT

        p = subprocess.run(
            [str(x) for x in cmd_list],
            cwd=str(cwd) if cwd else None,
            env=env,
            timeout=timeout_s,
            stdout=stdout,
            stderr=stderr,
            check=False,
        )
        if p.returncode != 0 and not allow_nonzero:
            raise RuntimeError(f"command failed rc={p.returncode}: {_print_cmd([str(x) for x in cmd_list])}")
        return p.returncode
    finally:
        if log_f is not None:
            log_f.close()



def load_events(path: Path):
    with open(path, "r", encoding="utf-8") as f:
        j = json.load(f)
    ev = j.get("traceEvents", j)
    if not isinstance(ev, list):
        raise TypeError("traceEvents is not a list")
    return ev


def summarize_be(events):
    ev = [e for e in events if e.get("ph") in ("B", "E") and "ts" in e]
    ev.sort(key=lambda e: (e.get("pid", 0), e.get("tid", 0), e.get("ts", 0), 0 if e.get("ph") == "B" else 1))

    stacks = defaultdict(list)  # (pid,tid) -> [(name,ts,child_us)]
    total_us = Counter()
    self_us = Counter()
    count = Counter()
    max_us = Counter()

    for e in ev:
        k = (e.get("pid", 0), e.get("tid", 0))
        ph = e.get("ph")
        name = e.get("name") or "(unnamed)"
        ts = int(e.get("ts", 0))

        st = stacks[k]
        if ph == "B":
            st.append([name, ts, 0])
            continue

        # E
        if not st:
            continue
        n0, ts0, child = st.pop()
        dur = ts - ts0
        if dur < 0:
            continue

        total_us[n0] += dur
        count[n0] += 1
        if dur > max_us[n0]:
            max_us[n0] = dur

        sd = dur - child
        if sd < 0:
            sd = 0
        self_us[n0] += sd

        if st:
            st[-1][2] += dur

    return {
        "be_events": len(ev),
        "total_us": total_us,
        "self_us": self_us,
        "count": count,
        "max_us": max_us,
    }


def scope_stats(summary, key: str):
    self_us = sum(v for n, v in summary["self_us"].items() if key in n)
    cnt = sum(v for n, v in summary["count"].items() if key in n)
    mx = max((v for n, v in summary["max_us"].items() if key in n), default=0)
    avg_us = (self_us / cnt) if cnt else 0.0
    return {"self_us": int(self_us), "cnt": int(cnt), "avg_us": float(avg_us), "max_us": int(mx)}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--build-dir", default=r"./build-cmake")
    ap.add_argument("--config", default="")
    ap.add_argument("--target", default="3d_screen_script")
    ap.add_argument("--iters", type=int, default=1, help="iterations per config")
    ap.add_argument("--out-dir", default=r"./tmp/traces")
    ap.add_argument("--warmup-ms", type=int, default=2000)
    ap.add_argument("--run-ms", type=int, default=5000)

    ap.add_argument("--nowait", action="store_true", help="set DONG_GPU_SWAPCHAIN_NOWAIT=1")
    ap.add_argument("--present-mode", choices=["mailbox", "vsync", "immediate"], default=None, help="set DONG_GPU_PRESENT_MODE")
    ap.add_argument("--frames-in-flight", type=int, default=None, help="set DONG_GPU_FRAMES_IN_FLIGHT (1..3)")
    ap.add_argument("--layer-cache", action="store_true", help="set DONG_LAYER_CACHE=1")
    ap.add_argument("--no-split-cmd-buf", action="store_true", help="set DONG_GPU_SPLIT_CMD_BUF=0")
    ap.add_argument("--gpu-build-draw-batches", action="store_true", help="set DONG_GPU_BUILD_DRAW_BATCHES=1")

    ap.add_argument("--sweep", action="store_true", help="auto-sweep present-mode/frames-in-flight/nowait combos")
    ap.add_argument("--no-build", action="store_true")
    ap.add_argument("--retries", type=int, default=1, help="retries per run if app fails")
    ap.add_argument("--timeout-grace-s", type=int, default=45, help="extra seconds beyond warmup+run")

    args = ap.parse_args()

    build_dir = Path(args.build_dir)
    out_dir = Path(args.out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    # Handle both Release subdir (MSVC) and flat (Ninja) layouts
    if args.config:
        exe = build_dir / args.config / f"{args.target}.exe"
    else:
        exe = build_dir / f"{args.target}.exe"
    if not exe.exists():
        alt = build_dir / f"{args.target}.exe"
        if alt.exists():
            exe = alt
    if not exe.exists():
        # Try Release subdir as fallback
        alt2 = build_dir / "Release" / f"{args.target}.exe"
        if alt2.exists():
            exe = alt2

    if not exe.exists():
        print(f"ERROR: cannot find exe: {exe}")
        return 2

    # Build once up-front unless user asks otherwise.
    if not args.no_build:
        run_checked(["cmake", "--build", str(build_dir), "--config", args.config, "--target", args.target, "--", "/m", "/v:m"])

    def make_env(cfg):
        env = os.environ.copy()
        # make runs comparable and avoid double limiting
        env.pop("DONG_FRAME_SLEEP_MS", None)
        env.pop("DONG_DEBUG_OFFSCREEN_WAIT_GPU_IDLE_FIRST_FRAME", None)

        env["DONG_BENCH_AUTOSTOP"] = "1"
        env["DONG_BENCH_WARMUP_MS"] = str(args.warmup_ms)
        env["DONG_BENCH_RUN_MS"] = str(args.run_ms)

        if cfg.get("nowait"):
            env["DONG_GPU_SWAPCHAIN_NOWAIT"] = "1"
        else:
            env.pop("DONG_GPU_SWAPCHAIN_NOWAIT", None)

        pm = cfg.get("present_mode")
        if pm:
            env["DONG_GPU_PRESENT_MODE"] = pm
        else:
            env.pop("DONG_GPU_PRESENT_MODE", None)

        fif = cfg.get("frames_in_flight")
        if fif is not None:
            env["DONG_GPU_FRAMES_IN_FLIGHT"] = str(fif)
        else:
            env.pop("DONG_GPU_FRAMES_IN_FLIGHT", None)

        if args.layer_cache:
            env["DONG_LAYER_CACHE"] = "1"
        else:
            env.pop("DONG_LAYER_CACHE", None)

        if args.no_split_cmd_buf:
            env["DONG_GPU_SPLIT_CMD_BUF"] = "0"

        if args.gpu_build_draw_batches:
            env["DONG_GPU_BUILD_DRAW_BATCHES"] = "1"
        else:
            env.pop("DONG_GPU_BUILD_DRAW_BATCHES", None)

        return env

    # Decide configs
    if args.sweep:
        # 默认 sweep：只比较阻塞 vs NOWAIT（这两个对 demo 的影响最大且最稳定）。
        # 其他参数（present-mode / frames-in-flight）目前仅部分路径使用，且可能受平台/后端支持限制。
        cfgs = [
            {"present_mode": None, "frames_in_flight": None, "nowait": False},
            {"present_mode": None, "frames_in_flight": None, "nowait": True},
        ]

    else:
        cfgs = [{
            "present_mode": args.present_mode,
            "frames_in_flight": args.frames_in_flight,
            "nowait": bool(args.nowait),
        }]

    stamp0 = datetime.now().strftime("%Y%m%d_%H%M%S")
    results = []

    for cfg_i, cfg in enumerate(cfgs):
        print("\n=== CONFIG", cfg_i, cfg, "===")
        sys.stdout.flush()

        for it in range(args.iters):
            stamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            cfg_tag = f"pm-{cfg.get('present_mode') or 'default'}_fif-{cfg.get('frames_in_flight') or 'default'}_nw-{1 if cfg.get('nowait') else 0}"
            trace = out_dir / f"{args.target}_{stamp0}_{cfg_tag}_i{it}.json"
            run_log = out_dir / f"{args.target}_{stamp0}_{cfg_tag}_i{it}.log"

            env = make_env(cfg)
            timeout_s = (args.warmup_ms + args.run_ms) / 1000.0 + float(args.timeout_grace_s)

            ok = False
            last_err = None
            last_rc = None
            for attempt in range(max(1, args.retries + 1)):
                try:
                    last_rc = run_checked(
                        [str(exe), "--profile", str(trace)],
                        env=env,
                        timeout_s=timeout_s,
                        log_path=run_log,
                        allow_nonzero=True,
                    )
                    if trace.exists() and trace.stat().st_size > 0:
                        ok = True
                        break
                    last_err = f"no trace written (rc={last_rc})"
                    print(f"WARN: run produced no trace attempt={attempt} rc={last_rc}")
                except Exception as e:
                    last_err = str(e)
                    print(f"WARN: run failed attempt={attempt}: {e}")
                time.sleep(0.5)


            if not ok:
                results.append({
                    "cfg": cfg,
                    "iter": it,
                    "trace": str(trace),
                    "log": str(run_log),
                    "status": "failed_run",
                    "rc": int(last_rc) if last_rc is not None else None,
                    "error": last_err,
                })

                print(f"RESULT cfg={cfg_tag} status=failed_run error={last_err} log={run_log}")
                sys.stdout.flush()
                continue

            # Parse trace for key metrics (no external tool dependency).
            try:
                events = load_events(trace)
                s = summarize_be(events)
            except Exception as e:
                results.append({
                    "cfg": cfg,
                    "iter": it,
                    "trace": str(trace),
                    "log": str(run_log),
                    "status": "failed_parse",
                    "error": str(e),
                })
                print(f"RESULT cfg={cfg_tag} status=failed_parse error={e} trace={trace}")
                sys.stdout.flush()
                continue


            k_wait_block = scope_stats(s, "SDL_WaitAndAcquireGPUSwapchainTexture")
            k_wait_nowait = scope_stats(s, "SDL_AcquireGPUSwapchainTexture")
            if k_wait_block["cnt"] > 0:
                k_wait = k_wait_block
                k_wait_mode = "wait"
            else:
                k_wait = k_wait_nowait
                k_wait_mode = "nowait" if k_wait_nowait["cnt"] > 0 else "unknown"

            k_submit = scope_stats(s, "SDL_SubmitGPUCommandBuffer")
            k_frame = scope_stats(s, "Frame")
            k_off_rebuild = scope_stats(s, "offscreen_rebuild")


            run_s = max(0.001, float(args.run_ms) / 1000.0)
            fps = (k_frame["cnt"] / run_s) if k_frame["cnt"] else 0.0
            wait_ms_avg = (k_wait["avg_us"] / 1000.0) if k_wait["cnt"] else 0.0

            row = {
                "cfg": cfg,
                "iter": it,
                "trace": str(trace),
                "log": str(run_log),
                "status": "ok",
                "rc": int(last_rc) if last_rc is not None else None,
                "events": len(events),
                "be_events": s["be_events"],
                "fps_est": fps,
                "wait_mode": k_wait_mode,
                "wait_acquire": k_wait,

                "submit": k_submit,
                "frame": k_frame,
                "offscreen_rebuild": k_off_rebuild,
            }

            results.append(row)

            print(
                f"RESULT cfg={cfg_tag} fps~{fps:.1f} "
                f"wait_mode={k_wait_mode} wait_avg_ms={wait_ms_avg:.2f} "
                f"submit_avg_ms={(k_submit['avg_us']/1000.0 if k_submit['cnt'] else 0.0):.2f} "
                f"offscreen_rebuild_avg_ms={(k_off_rebuild['avg_us']/1000.0 if k_off_rebuild['cnt'] else 0.0):.2f}"
            )

            sys.stdout.flush()

            time.sleep(0.2)

    # Pick best config: minimize wait-acquire self time, then maximize fps.
    def score(r):
        return (r["wait_acquire"]["self_us"], -r["fps_est"], r["submit"]["self_us"], r["offscreen_rebuild"]["self_us"])

    ok_results = [r for r in results if isinstance(r, dict) and r.get("wait_acquire")]
    best = min(ok_results, key=score) if ok_results else None


    out_json = out_dir / f"results_{args.target}_{stamp0}.json"
    with open(out_json, "w", encoding="utf-8") as f:
        json.dump({"stamp": stamp0, "target": args.target, "args": vars(args), "results": results, "best": best}, f, ensure_ascii=False, indent=2)

    print("\n=== DONE ===")
    print(f"wrote: {out_json}")
    if best is not None:
        print(f"BEST cfg={best['cfg']} fps~{best['fps_est']:.1f} wait_self_us={best['wait_acquire']['self_us']} trace={best['trace']}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
