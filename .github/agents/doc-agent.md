# Doc-Agent

## Purpose

The Doc-Agent inspects pushed or modified documentation inputs from the main repository and prepares wiki-ready Markdown for the next stage.

## Inputs

The initial implementation only reviews content that exists in the main repository:

- `README.md`
- `QT_Project/**`
- `QT_Project_Tests/**`
- `Configs/**`

## Responsibilities

- Detect whether one or more monitored documentation inputs changed.
- Review the changed inputs in a deterministic way.
- Prepare wiki-ready Markdown in a staging area.
- Supplement the wiki staging content when useful and safe.
- Keep the generated content focused and minimal.

## Current Output

The Doc-Agent prepares these wiki staging files:

- `Home.md`
- `Documentation-Review.md`
- `_Sidebar.md`

## Rules

- Do not update the GitHub Wiki directly.
- Do not push to the wiki repository.
- Do not introduce GitHub Pages in the initial implementation.
- Do not depend on `docs/**` or `CHANGELOG.md` for the initial version.
- Keep the workflow deterministic and easy to review.

## Notes

The first implementation is intentionally simple:

- `Home.md` is derived from `README.md`
- `Documentation-Review.md` summarizes the reviewed inputs and changed files
- `_Sidebar.md` provides minimal wiki navigation