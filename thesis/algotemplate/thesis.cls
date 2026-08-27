\NeedsTeXFormat{LaTeX2e}
% define the class thesis
\ProvidesClass{algotemplate/thesis}

% ---------- key value options for classes ----------
\RequirePackage{kvoptions}
\SetupKeyvalOptions{
    family=kn,
    prefix=kn@
}
\DeclareStringOption[bibliography]{bibfile}
\DeclareStringOption[abbrvurl]{bibstyle}
\DeclareStringOption[1]{openuptoc}
\def\kn@defaulttarget{print}
\expandafter\DeclareStringOption\expandafter[\kn@defaulttarget]{target}
\DeclareBoolOption{chapters}
\DeclareDefaultOption{
    \expandafter\PassOptionsToClass\expandafter{\CurrentOption}{scrreprt}
}
\ProcessKeyvalOptions* % process options

\ifx\kn@target\kn@defaulttarget
    \PassOptionsToClass{twoside}{scrreprt}
\else
    \PassOptionsToClass{oneside}{scrreprt}
\fi
\LoadClass[10pt,a4paper,parskip,chapterprefix=true]{scrreprt} % load default page settings and document class

\RequirePackage{algotemplate/format} % Load formatting of general stuff
\RequirePackage{babel}               % set language setting
\RequirePackage{setspace}            % for titlepage
\RequirePackage[top=25mm,bottom=3cm]{geometry}

% --------------------------------- Thesis specific formatting ---------------------------------

% --------- Single/Double sided printing ---------
\ifx\kn@target\kn@defaulttarget
    \geometry{inner=3cm,outer=2cm}  % 1cm binding correction

    \fancyhead[RO]{\includegraphics[width=60pt]{figures/algo_see.pdf}\vspace{20pt}}
    \fancyfoot[LE,RO]{
        \vspace{20pt}
        \begin{tcolorbox}[height=40pt,hbox,sharp corners,boxsep=2pt,left=0pt,right=0pt,top=2pt,colback=kn_seeblau,colframe=kn_seeblau,coltext=white]
            \thepage
        \end{tcolorbox}
    }
    \fancypagestyle{intro}{
        \fancyhf{}
        \fancyhead[RO]{\includegraphics[width=60pt]{figures/algo_see.pdf}\vspace{20pt}}
        \renewcommand{\headrulewidth}{0pt} % clear lines in header and footer
        \renewcommand{\footrulewidth}{0pt}
        \fancyfoot[LE,RO]{
            \vspace{20pt}
            \begin{tcolorbox}[height=40pt,hbox,sharp corners,boxsep=2pt,left=0pt,right=0pt,top=2pt,colback=kn_seeblau,colframe=kn_seeblau,coltext=white]
                \thepage
            \end{tcolorbox}
        }
    }

    \AddToHook{begindocument/before}[algotemplate]{\hypersetup{colorlinks=false}}
\else
    % In this case, the flag for single sided layout is set
    \geometry{left=25mm,right=25mm}

    \fancyhead[R]{\includegraphics[width=60pt]{figures/algo_see.pdf}\vspace{20pt}}
    \fancyfoot[R]{
        \vspace{20pt}
        \begin{tcolorbox}[height=40pt,hbox,sharp corners,boxsep=2pt,left=0pt,right=0pt,top=2pt,colback=kn_seeblau,colframe=kn_seeblau,coltext=white]
            \thepage
        \end{tcolorbox}
    }
    \fancypagestyle{intro}{
        \fancyhf{}
        \fancyhead[R]{\includegraphics[width=60pt]{figures/algo_see.pdf}\vspace{20pt}}
        \renewcommand{\headrulewidth}{0pt} % clear lines in header and footer
        \renewcommand{\footrulewidth}{0pt}
        \fancyfoot[R]{
            \vspace{20pt}
            \begin{tcolorbox}[height=40pt,hbox,sharp corners,boxsep=2pt,left=0pt,right=0pt,top=2pt,colback=kn_seeblau,colframe=kn_seeblau,coltext=white]
                \thepage
            \end{tcolorbox}
        }
    }

    % make \cleardoublepage behave like \clearpage
    \let\cleardoublepage=\clearpage
\fi

% --------- Header and footer ---------
\fancyhf{}                         % clear header and footer
\renewcommand{\headrulewidth}{0pt} % clear lines in header and footer
\renewcommand{\footrulewidth}{0pt}
\setlength\headheight{25mm}        % set height of header
\addtolength{\textheight}{-25mm}

% --------- Title page --------- (Please do not change - title page is fixed from the university)
\newcommand{\studentno}[1]{\def\@studentno{#1}}
\newcommand{\@studentno}{\ClassError{algotemplate/thesis}{No studentno specified}{Use e.g. \protect\studentno{123456}.}}
\newcommand{\institute}[1]{\def\@institute{#1}}
\newcommand{\@institute}{}
\newcommand{\department}[1]{\def\@department{#1}}
\newcommand{\@department}{\ClassError{algotemplate/thesis}{No department specified}{Use e.g. \protect\department{Computer and Information Science}.}}
\newcommand{\group}[1]{\def\@group{#1}}
\newcommand{\@group}{\ClassError{algotemplate/thesis}{No group specified}{Use e.g. \protect\group{Algorithmics and Theoretical Computer Science}.}}
\newcommand{\type}[1]{\def\@type{#1}}
\newcommand{\@type}{\ClassError{algotemplate/thesis}{No type specified}{Use e.g. \protect\type{Master Thesis}.}}
\newcommand{\advisor}[1]{\def\@advisor{#1}}
\newcommand{\@advisor}{\ClassError{algotemplate/thesis}{No advisor specified}{Use e.g. \protect\advisor{Prof.~Dr.~Sabine Storandt}.}}
\newcommand{\reviewer}[1]{\def\@reviewer{#1}}
\newcommand{\@reviewer}{\ClassError{algotemplate/thesis}{No reviewer specified}{Use e.g. \protect\reviewer{Prof.~Dr.~Sabine Storandt}.}}

\renewcommand{\maketitle}{
\begin{titlepage}
    \pagenumbering{Alph}
    \begin{center}
        {\LARGE\sffamily\bfseries\setstretch{1.2}{\@title}\par}\vspace{1.5cm}
        {\Large\sffamily{\@type}}\\[1.5cm]
        {\large\sffamily submitted by\\[.5cm]
        \Large\@author\ (\@studentno)}\\[.5cm]
        {\large\sffamily at the}
        \vfill
        \resizebox{.6\linewidth}{!}{\includegraphics{figures/unisignet.pdf}}\\[.4cm]
        \vfill
        {\large\sffamily\@group~}\\[.5cm]
        {\large\sffamily Department of \@department}\\[1cm]
        {\large\sffamily\begin{tabular}{rl}
        Advisor: & \@advisor\\[.5cm]
        Reviewer: & \@reviewer
        \end{tabular}}\\[1cm]
        {\large\sffamily Konstanz, \@date}
    \end{center}
\end{titlepage}
}

% --------- New format for abstract ---------
\long\def\abstract#1\end#2{\gdef\kn@abstract{#1}\end{#2}}
\long\def\endabstract{}
\gdef\kn@abstract{\ClassError{algotemplate/thesis}{No abstract specified}{Use e.g. \protect\begin{abstract} ... \protect\end{abstract}.}}
\newcommand{\makeabstract}{
    \pagenumbering{roman}
    \pagestyle{intro}
    \phantomsection
    \addcontentsline{toc}{section}{Abstract}
    \section*{Abstract}
    {\kn@abstract}
}

% --------- New format for sections, chapters ---------
\setkomafont{sectioning}{\sffamily\LARGE}
\addtokomafont{section}{\sffamily\LARGE\sectionuline}
\addtokomafont{subsection}{\sffamily}
\addtokomafont{subsubsection}{\sffamily}
\setcounter{tocdepth}{2} % show sections, subsections and subsubsections in toc
\setcounter{secnumdepth}{3} % show section numbering up to 3 numberings, eg 1.2.1

\newcommand{\chapbox}[2]{
        \begin{minipage}[t]{\textwidth}
            \begin{tcolorbox}
                [sharp corners,
                left,
                hbox,
                parbox=false,
                colback=kn_seeblau35,
                boxrule=0pt,
                enhanced,
                left=2mm,
                top=2mm,
                right=2mm,
                fontupper=\strut,
                bottom=2mm,
                colframe=white]
                \Huge\bfseries\sffamily
                \chapapp~#1
            \end{tcolorbox}\vspace{-1pt}
            \begin{tcolorbox}
                [sharp corners,
                left,
                hbox,
                parbox=false,
                colback=kn_seeblau65,
                fontupper=\strut,
                boxrule=0pt,
                enhanced,
                left=2mm,
                top=2mm,
                right=2mm,
                bottom=2mm,
                colframe=white]
                \Huge\bfseries\sffamily
                #2
            \end{tcolorbox}
        \end{minipage}
}

\ifkn@chapters
    \renewcommand\thechapter{\arabic{chapter}}
    \renewcommand\thesection{\thechapter.\arabic{section}}
    \renewcommand\thesubsection{\thesection.\arabic{subsection}}
    \renewcommand\thesubsubsection{\thesubsection.\arabic{subsubsection}}
    \addtokomafont{chapter}{\sffamily}
    % spacing for chapters and sections in TOC
    \pretocmd{\chapter}{\addtocontents{toc}{\protect\addvspace{15\p@}}}{}{}
    \pretocmd{\section}{\addtocontents{toc}{\protect\addvspace{5\p@}}}{}{}
    \pretocmd{\subsection}{\addtocontents{toc}{\protect\addvspace{2.5\p@}}}{}{}

    \renewcommand{\chapterlineswithprefixformat}[3]{%
        \ifnum\value{chapter}=0%
            \sectionuline{#3}%
        \else%
            \chapbox{\thechapter}{#3}%
        \fi%
    }
\else
    \setkomafont{sectioning}{\sffamily\LARGE\sectionuline}
    \addtokomafont{subsubsection}{\rule[0.5ex]{1.5em}{0.5pt}\hskip1em\relax}
    \renewcommand\thesection{\arabic{section}}
    \renewcommand\thesubsection{\thesection.\arabic{subsection}}
    \renewcommand\thesubsubsection{\thesubsection.\arabic{subsubsection}}
\fi

% sections have blue underlinings
\newcommand\sectionuline{%
    \bgroup\markoverwith{\textcolor{kn_seeblau}%
    {\rule[\dimexpr-4pt-2pt\relax]{2pt}{2pt}}}\ULon
}

% --------- Generate Intro ---------
\AddToHook{begindocument/end}[algotemplate]{
    \addtocontents{toc}{\protect\thispagestyle{intro}}
    \maketitle
    \cleardoublepage
    \makeabstract
    \cleardoublepage
    {\hypersetup{allcolors=black}\openup\kn@openuptoc\jot\tableofcontents}
    \cleardoublepage
    \pagenumbering{arabic}
}

% --------- Generate References ---------
\AddToHook{enddocument}[algotemplate]{
    \fancypagestyle{plain}{\pagestyle{fancy}}
    \thispagestyle{fancy}
    %\renewcommand{\bibname}{References}

    %for chapters
    \renewcommand{\refname}{References}

    \renewcommand{\bibsection}{%
        \section*{\refname%
        }
    }
    % end for chapters
    \cleardoublepage
    \phantomsection
    \addcontentsline{toc}{section}{References}
    \bibliography{\kn@bibfile}
    \bibliographystyle{\kn@bibstyle}
}

% --------- Page numbers ---------
\addtokomafont{pagenumber}{\small\sffamily}

% Add back links
\patchcmd{\BR@backref}{\newblock}{\newblock[page~}{}{}
\patchcmd{\BR@backref}{\par}{]\par}{}{}

% --------- numbering appearance ---------
\renewcommand{\thefigure}{\arabic{figure}}
\renewcommand{\thetable}{\arabic{table}}
