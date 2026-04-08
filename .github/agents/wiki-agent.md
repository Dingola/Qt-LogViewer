# Wiki-Agent

## Purpose

The Wiki-Agent is the only automation component allowed to update the GitHub Wiki.

## Inputs

The Wiki-Agent consumes only reviewed and approved staging content from the main repository:

- `.github/wiki-staging/Home.md`
- `.github/wiki-staging/Documentation-Review.md`
- `.github/wiki-staging/_Sidebar.md`
- `.github/wiki-staging/.wiki-managed-files`

## Responsibilities

- Read the approved staging content from the main repository.
- Clone the GitHub Wiki repository.
- Publish only the pages listed in `.wiki-managed-files`.
- Preserve unrelated wiki pages that are not managed by the automation.
- Commit and push only when the wiki content actually changed.

## Rules

- Do not inspect repository documentation inputs directly as a primary task.
- Do not perform documentation analysis that belongs to the Doc-Agent.
- Do not introduce GitHub Pages in the initial implementation.
- Do not overwrite unrelated manual wiki pages.
- Publish only reviewed and approved staging content from the main repository.
- Keep commits deterministic and minimal.

## Notes

GitHub Wikis are stored in a separate Git repository internally.

The Wiki-Agent may clone and push that repository during workflow execution, but it must treat the main repository staging content as the reviewed source of truth for automation-owned wiki pages.