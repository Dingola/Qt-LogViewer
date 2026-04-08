# Doc-Agent

## Purpose

The Doc-Agent inspects pushed or modified documentation inputs from the main repository and prepares reviewable wiki staging content.

## Inputs

The initial implementation reviews content that exists in the main repository:

- `README.md`
- `QT_Project/**`
- `QT_Project_Tests/**`
- `Configs/**`

## Responsibilities

- Detect whether one or more monitored documentation inputs changed.
- Review the changed inputs in a deterministic way.
- Prepare wiki-ready Markdown in `.github/wiki-staging/`.
- Supplement the staging content when useful and safe.
- Keep the generated content focused and minimal.
- Create or update a pull request so the prepared staging content can be reviewed before publication.

## Current Output

The Doc-Agent prepares these staging files:

- `.github/wiki-staging/Home.md`
- `.github/wiki-staging/Documentation-Review.md`
- `.github/wiki-staging/_Sidebar.md`
- `.github/wiki-staging/.wiki-managed-files`

## Rules

- Do not update the GitHub Wiki directly.
- Do not push to the wiki repository.
- Do not introduce GitHub Pages in the initial implementation.
- Do not depend on `docs/**` or `CHANGELOG.md` for the initial version.
- Keep the workflow deterministic and easy to review.
- Write prepared wiki content into the main repository in a reviewable staging location before any wiki publishing occurs.

## Notes

The first implementation is intentionally simple:

- `Home.md` is derived from `README.md`
- `Documentation-Review.md` summarizes the reviewed inputs and changed files
- `_Sidebar.md` provides minimal wiki navigation
- `.wiki-managed-files` defines the exact wiki pages owned by the automation