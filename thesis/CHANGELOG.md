# Changelog

## v3.0.0

### Features

*   option `bibstyle`
*   proper print option
    *   binding correction
    *   no colored links
    *   `\clearpage` means `\cleardoublepage`

### Changes

*   remove option `twoside` in favor of `target=print`
*   delete knnat in favor of abbrvurl
*   recommend `\cref` instead of `\ref`
*   merge conclusion and outlook in manual
*   add isodate
*   `\DontPrintSemicolon`
*   color links in dark seeblau
*   different syntax for abstract
    (`\begin{abstract}...\end{abstract}` instead of `\abstract{...}`)
*   move instructions from main.tex to README.md

### Fixes

*   place TOC entries for abstract and bibliography on their first pages
    (not last)
*   better error messages for missing fields on title page
*   style headings of subsubsections according to corporate design
