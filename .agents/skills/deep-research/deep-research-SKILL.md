---
name: deep-research
description: Deep research agent - comprehensive multi-source research, analysis, and synthesis on any topic. Use when user asks to research, investigate, or deeply analyze a topic.
allowed-tools: WebSearch, WebFetch, Read, Glob, Grep, Agent, Bash, Skill, CronCreate, CronDelete, CronList, mcp__________create_doc, mcp__________edit_doc_content
---

# Deep Research

Perform comprehensive, multi-step research on a topic, synthesizing information from multiple sources into a well-structured report.

## When to Trigger

- User says "研究", "调研", "deep research", "investigate", "分析一下", "帮我查", "深入了解"
- User asks a complex question that requires multi-source synthesis
- User wants a report or analysis on a topic

## Workflow

### Phase 1: Understand & Plan
1. Parse the research question/topic
2. Break it into 3-8 sub-questions that cover the topic comprehensively
3. Send a brief plan to the user via `pipi-ipc` skill so they know what's coming

### Phase 2: Parallel Research
4. Launch multiple search queries in parallel using `WebSearch`
5. For each promising result, use `WebFetch` to get detailed content
6. For code/technical topics, also search the local codebase with `Grep`/`Glob`
7. Use `Agent` (subagent_type: "general-purpose") for deep-dive sub-topics that need their own multi-step research

### Phase 3: Synthesize
8. Cross-reference findings from multiple sources
9. Identify consensus, contradictions, and gaps
10. Prioritize recent and authoritative sources

### Phase 4: Deliver
11. Write a structured report with:
    - **Executive Summary** (1-3 sentences)
    - **Key Findings** (bullet points)
    - **Detailed Analysis** (organized by sub-topic)
    - **Sources** (with URLs)
    - **Open Questions / Further Research** (if applicable)
12. **Always create a WeCom doc** (`create_doc` + `edit_doc_content`) for the full report
13. Send the doc link to the user via `pipi-ipc` skill, with a brief summary (3-5 bullet points)

## Research Quality Rules

- **Minimum 3 sources** per key claim
- **Prefer recent sources** (within last 12 months) unless historical context needed
- **Always include source URLs** - never make claims without attribution
- **Acknowledge uncertainty** - clearly state when evidence is limited or conflicting
- **Use the user's language** - if they ask in Chinese, report in Chinese; English question = English report
- **Be opinionated** - after presenting evidence, give your assessment/recommendation
- **Cite publication dates** when available to help user judge freshness

## Output Format

For chat delivery, use this structure:

```
## [Topic] 调研报告

### 摘要
[1-3 sentence summary]

### 关键发现
- Finding 1
- Finding 2
- Finding 3

### 详细分析

#### [Sub-topic 1]
[Analysis with inline source links]

#### [Sub-topic 2]
[Analysis with inline source links]

### 来源
- [Source Title](URL) - brief note
- [Source Title](URL) - brief note

### 待深入研究
- Open question 1
- Open question 2
```

## Tips

- For technical topics: combine web research with codebase analysis
- For product/market topics: look for official announcements, blog posts, and HN/Reddit discussions
- For academic topics: search for papers, surveys, and expert blog posts
- Send progress updates via `pipi-ipc` skill for long research (>2 min)
- If the topic is too broad, ask the user to narrow down before diving in
