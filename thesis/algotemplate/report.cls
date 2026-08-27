\NeedsTeXFormat{LaTeX2e}
% define the class thesis
\ProvidesClass{algotemplate/report}

% ---------- key value options for classes ----------
\RequirePackage{kvoptions}
\SetupKeyvalOptions{
    family=kn,
    prefix=kn@
}
\DeclareStringOption{bibfile}
\DeclareStringOption[abbrvurl]{bibstyle}
\DeclareBoolOption{twocolumn}
\DeclareDefaultOption{
    \expandafter\PassOptionsToClass\expandafter{\CurrentOption}{article}
}
\ProcessKeyvalOptions* % process options
\newif\ifkn@chapters
\newcounter{chapter}

\LoadClass{article}

\RequirePackage{titlesec}
\RequirePackage{xstring}

% Load formatting of general stuff
\RequirePackage{algotemplate/format}

% set language setting
\RequirePackage{babel}

\newcommand{\institute}[1]{\def\kn@institute{#1}}

% DEAD
\newcommand{\studentno}[1]{}
\newcommand{\department}[1]{}
\newcommand{\group}[1]{}
\newcommand{\type}[1]{}
\newcommand{\advisor}[1]{}
\newcommand{\reviewer}[1]{}

% --------- Page settings and line breaks ---------
\parskip=10pt plus1pt minus1pt
\parindent=0pt
\usepackage[margin=2cm, top=2cm, bottom=2cm, headheight=0pt, headsep=0pt]{geometry}

% --------- Generate References ---------
\ifx\kn@bibfile\empty
\else
    \AtEndDocument{
        \bibliography{\kn@bibfile}
        \bibliographystyle{\kn@bibstyle}
    }
\fi

% --------- Abstract and title design ---------
\newcommand\sectionuline{%
    \bgroup\markoverwith{\textcolor{kn_seeblau}{\rule[\dimexpr-4pt-2pt\relax]{2pt}{2pt}}}%
    \ULon
}

\titleformat{\section}
{\normalfont\Large\bfseries\raggedright}
{\sectionuline{\thesection.\enspace}}
{-0.15em}
{\sectionuline}

\titleformat{\subsection}
{\normalfont\large\bfseries\raggedright}
{\sectionuline{\thesubsection.\enspace}}
{-0.15em}
{\sectionuline}

% --------- New title command for blue boxing around title ---------
\newcommand{\cheatBoxHeight}{\makebox[0pt][l]{\phantom{A}}\makebox[0pt][l]{\phantom{y}}}

\newcommand{\kntitleformatted}[3]{%
    \noindent
    \def\paramthree{#3}%
    \def\paramtwo{#2}%
    \ifx\paramthree\empty%
        \ifx\paramtwo\empty%
            % only #1 is filled
            \colorbox{kn_seeblau65}{\LARGE\bf\fontfamily{lmss}\selectfont #1\cheatBoxHeight}
        \else%
            % #1 and #2 are filled
            \colorbox{kn_seeblau35}{\LARGE\bf\fontfamily{lmss}\selectfont #1\cheatBoxHeight}\\\lineskip=0pt
            \colorbox{kn_seeblau65}{\LARGE\bf\fontfamily{lmss}\selectfont #2\cheatBoxHeight}
        \fi%
    \else%
        % all arguments are filled
        \colorbox{kn_seeblau20}{\LARGE\bf\fontfamily{lmss}\selectfont #1\cheatBoxHeight}\\\lineskip=0pt
        \colorbox{kn_seeblau35}{\LARGE\bf\fontfamily{lmss}\selectfont #2\cheatBoxHeight}\\\lineskip=0pt
        \colorbox{kn_seeblau65}{\LARGE\bf\fontfamily{lmss}\selectfont #3\cheatBoxHeight}
    \fi%
}

% --------- Abstract and title design ---------
\long\def\abstract#1\end#2{\gdef\kn@abstract{#1}\end{#2}}
\long\def\endabstract{}

\renewcommand{\maketitle}{
    % count the number of line breaks
    \StrCount{\@title}{\\}[\numlinebreaks]

    \ifnum\numlinebreaks=0
        % title has one line
        \kntitleformatted{\@title}{}{}
    \else
        \ifnum\numlinebreaks=1
            % title has two lines

            % get rid of spaces before and after line break
            \StrSubstitute{\@title}{ \\}{\\}[\@title]
            \StrSubstitute{\@title}{\\ }{\\}[\@title]
            % parse title
            \StrBefore{\@title}{\\}[\firstline] % extract the first line
            \StrBehind{\@title}{\firstline\\}[\secondline] % extract the remaining lines
            % call title generator
            \kntitleformatted{\firstline}{\secondline}{}
        \else
            % title has more than 3 lines

            % get rid of spaces before and after line break
            \StrSubstitute{\@title}{ \\}{\\}[\@title]
            \StrSubstitute{\@title}{\\ }{\\}[\@title]
            % parse title
            \StrBefore{\@title}{\\}[\firstline] % extract the first line
            \StrBehind{\@title}{\firstline\\}[\remaining] % extract the remaining lines
            \StrBefore{\remaining}{\\}[\secondline] % extract the second line
            \StrBehind{\remaining}{\secondline\\}[\thirdline] % extract the third line
            \StrSubstitute{\thirdline}{\\}{ }[\thirdline]
            % call title generator
            \kntitleformatted{\firstline}{\secondline}{\thirdline}
        \fi
    \fi
}

% --------- Generate Intro ---------
\AfterEndPreamble{%
    \pagestyle{fancy}%
    \ifkn@twocolumn
        \twocolumn[
            \maketitle
            \ifx\kn@abstract\undefined
            \else
                \section*{\abstractname}
                \kn@abstract
            \fi
        \vskip 3em]%
    \else
        \maketitle
        \ifx\kn@abstract\undefined
        \else
            \section*{\abstractname}
            \kn@abstract
        \fi
        \vskip 3em%
    \fi
}%

% --------- Header and Footer ---------
\fancyhf{} % clear header and footer
\fancyfoot[L]{\fontfamily{lmss}\color{black} \thepage}
\fancyfoot[R]{\fontfamily{lmss}\color{black} -- \@author{} \ifx\kn@institute\undefined\else-- \kn@institute\fi{} -- \@date}
\renewcommand{\headrulewidth}{0pt} % clear lines in header and footer
\renewcommand{\footrulewidth}{1.6pt}
\renewcommand{\footrule}{\hbox to\headwidth{%
    \color{kn_seeblau}\leaders\hrule height \footrulewidth\hfill}
}
