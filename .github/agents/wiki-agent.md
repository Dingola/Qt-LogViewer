# Wiki-Agent

## Purpose

The Wiki-Agent is the only automation component allowed to update the GitHub Wiki.

## Inputs

The Wiki-Agent consumes the wiki staging content prepared by the Doc-Agent:

- `Home.md`
- `Documentation-Review.md`
- `_Sidebar.md`

## Responsibilities

- Clone the GitHub Wiki repository.
- Replace wiki content with the prepared staging content.
- Commit and push only when the wiki content actually changed.
- Keep the wiki synchronized with the Doc-Agent output.

## Rules

- Do not inspect repository documentation inputs directly as a primary task.
- Do not perform documentation analysis that belongs to the Doc-Agent.
- Do not introduce GitHub Pages in the initial implementation.
- Do not generate unrelated wiki pages.
- Keep commits deterministic and minimal.

## Required Secret

The workflow requires:

- `WIKI_PUSH_TOKEN`

This token must have sufficient permissions to push to the repository wiki.

## Notes

GitHub Wikis are stored in a separate Git repository internally.  
The Wiki-Agent may clone and push that repository during workflow execution, but the main project should still treat the wiki update as an automated publishing step only.