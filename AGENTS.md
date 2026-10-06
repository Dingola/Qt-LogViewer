# Repository Instructions

## Scope

- Follow these instructions for every change in this repository.
- Preserve existing conventions unless this file explicitly states otherwise.
- Keep changes focused on the requested task and avoid unrelated refactoring.

## C++ Control Flow

- A function returning a value must contain exactly one `return`, located at the end of the
  function.
- A `void` function must not use `return;` for early exits.
- Do not use `break` or `continue` in loops.
- `break` is permitted in `switch` statements.
- Express conditional control flow through structured conditions and local state.

## Documentation

- Document new and modified C++ declarations, definitions, helper functions, and tests with
  Doxygen comments.
- Keep Doxygen comments correctly indented.
- Include applicable `@brief`, `@param`, and `@return` tags.
- Write useful documentation separately in headers and source files.
- Do not use `@copydoc`.

## Existing Changes

- Preserve unrelated user changes.
- Do not stage, unstage, commit, discard, or overwrite changes unless explicitly requested.
- Do not include unrelated generated files, translations, or formatting changes in a proposed
  commit.

## Editing and Formatting

- Use the repository's existing formatting configuration for modified source files.
- Follow the established naming, layout, and architectural conventions.
- Avoid introducing new dependencies without a clear requirement.

## Verification

- Build the affected targets after implementation.
- Run focused tests covering the changed behavior.
- Run the complete test suite when practical.
- Run `git diff --check` before handing off changes.
- Report any verification that was skipped, failed, or could not be completed.

## Commits

- Follow the repository's existing Conventional Commit style.
- Base proposed commit messages on the actual staged diff.
- Do not create commits unless explicitly requested.
