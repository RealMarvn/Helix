# algotemplate

## How to Use

```tex
\documentclass[...]{algotemplate/thesis}

\title{...}
\author{...}
\studentno{...}
\group{...}
\department{...}
\institute{...}
\advisor{...}
\reviewer{...}
\date{...}

\begin{abstract}
    ...
\end{abstract}

\begin{document}

...

\end{document}
```

## Class Options

*   `target=print` or `target=digital` (thesis only):
    Optimize for viewing on paper or screen.
    `target=digital` changes `\cleardoublepage` to mean the same as `\clearpage`;
    this means that saying `\cleardoublepage` before each new section
    will not insert empty pages into the print version.
    (default: `target=print`)
*   `chapters=false` or `chapters=true` (thesis only):
    Use chapters as top-level structure.
    Recommended only for dissertations.
    Instead of `chapters=true`, you may type `chapters`.
    (default: `chapters=false`)
*   `english`, `ngerman`, …:
    Your advisor's favorite language.
    (default: `english`)
*   `bibfile=myfile.bib`:
    Name of your bibliography file.
    If missing, no bibliography is generated.
*   `bibstyle=mystyle.bst`:
    Name of your advisor's favorite bibliography style.
    (default: `bibstyle=abbrvurl`)
*   `twocolumn=false` or `twocolumn=true` (report only):
    Set the text in two columns per page.
    Instead of `twocolumn=true`, you may type `twocolumn`.
    (default: `twocolumn=false`)
*   `openuptoc` (thesis only):
    How much should the space between lines in the table of contents
    be increased, in units of 3 pt.
    (default: `openuptoc=1`)

## Included Packages

These packages are already included for your convenience:

*   `algorithm2e` with options `vlined,ruled,linesnumbered`
    and `\DontPrintSemicolon`
*   `amsmath`, `amssymb`, `amsthm`
*   `babel`
*   `cleveref`
*   `csquotes`
*   `graphicx`
*   `hyperref`
*   `inputenc` with option `utf8`
*   `isodate` with option `iso`
*   `natbib` with option `numbers`
*   `tikz`
*   …and more that are used interally.
    For a complete list, read the sources files.
