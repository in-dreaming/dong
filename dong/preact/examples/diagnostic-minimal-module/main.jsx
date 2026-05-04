// Minimal module test - no dependencies, just DOM manipulation
const root = document.getElementById('root');

if (root) {
    root.innerHTML = `
        <div style="
            width: 100%;
            height: 100%;
            display: flex;
            justify-content: center;
            align-items: center;
            background: #f5f5f5;
        ">
            <div style="
                width: 400px;
                padding: 40px;
                background: white;
                border-radius: 8px;
                box-shadow: 0 2px 8px rgba(0,0,0,0.1);
                text-align: center;
            ">
                <h1 style="color: #1a1a1a; margin-bottom: 20px;">
                    ✅ Module Script Works!
                </h1>
                <p style="color: #666; margin-bottom: 16px;">
                    If you see this, ES modules are supported.
                </p>
                <div style="display: flex; gap: 8px; justify-content: center;">
                    <button style="
                        padding: 10px 16px;
                        background: #0066cc;
                        color: white;
                        border: none;
                        border-radius: 4px;
                        cursor: pointer;
                    ">Module Test</button>
                </div>
            </div>
        </div>
    `;
    console.log("✅ Module loaded and executed");
} else {
    console.error("❌ root not found");
}
