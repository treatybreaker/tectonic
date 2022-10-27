# How To: Use Tectonic with AucTeX

This section is a guide aimed at [GNU
Emacs](https://www.gnu.org/software/emacs/) users for setting up
[AucTeX](https://www.gnu.org/software/auctex/) with Tectonic as the TeX/LaTeX
distribution.

## Basic Prerequisites

To follow this section you will need Tectonic and GNU Emacs installed on your
system. Additionally, you will require the AucTeX emacs package to be installed
before following along.

> Note: This section makes use of the [V2 tectonic CLI](../../ref/v2cli.md),
> invoked using the `tectonic -X` flag or `nextonic` command alias.

## Setup

All the code displayed in this section should go into your `init.el` file (or
equivalent, such as `config.el` if you are using
[Doomemacs](https://github.com/doomemacs/)).

First, load the AucTeX package.

```lisp
(require 'latex)
```

You will need to set the default TeX engine AucTeX uses to figure out the build
commands to use Tectonic instead of traditional TeX distributions. Therefore we
have to modify the `TeX-engine-alist` varible.
* The first element of the list is the symbol that AucTeX recognizes.
* The second element is a string with the name of the TeX distribution.
* The third element is the shell command for compiling plain TeX documents.
* The fourth element is the shell command for compiling LaTeX documents. Here we
  are assuming the user is using a Tectonic project (generated using `tectonic -X
  new <proj-name>`).
* The last element is the shell command for compiling ConTeXt documents, left
  unconfigured for now.

```lisp
(setq TeX-engine-alist '((default
                          "Tectonic"
                          "tectonic -X compile -f plain %T"
                          "tectonic -X watch"
                          nil)))
```

Next, modify the `LaTeX-command-style` so that AucTex doesn't add extra options
to it that Tectonic does not recognize. We simply set it to the `%(latex)`
expansion (from `TeX-expand-list-builtin`), removing any other extra options.

```lisp
(setq LaTeX-command-style '(("" "%(latex)")))
```

We need to set the `TeX-check-TeX` variable to `nil` since AucTeX will try to
find a traditional distibution like `TeXLive` or others, and will fail since
Tectonic doesn't meet it's criteria.

Additionally, we should also set `TeX-process-asynchronous` to `t`, so that
running Tectonic in watch mode doesn't hang up Emacs.

We'll also just ensure that the `TeX-engine` is set to `default`.

```lisp
(setq TeX-process-asynchronous t
      TeX-check-TeX nil
      TeX-engine 'default)
```

Finally, modify the `TeX-command-list` to use the appropriate commands, and not
pass in extra metadata and options to Tectonic which cause it to error out. This
needs to be done in place.

```lisp
(let ((tex-list (assoc "TeX" TeX-command-list))
      (latex-list (assoc "LaTeX" TeX-command-list)))
  (setf (cadr tex-list) "%(tex)"
        (cadr latex-list) "%l"))
```

And that is all! You should now be able to
1. Compile plain TeX files.
2. Build Tectonic LaTeX projects in watch mode.

## Additional Configuration and Usage Suggestions

### Compile LaTeX outside a Tectonic project

To do this, you can simply invoke `M-x TeX-command-master`, and the select the
`Other` option, passing in the compile command `tectonic -X compile -f latex <name
of file>`.

> **Caution**: Compiling a document with multiple LaTeX files in this manner
> isn't extensively tested, as using a Tectonic project is the better way in
> that case. Any bug reports are welcome.

### Live PDF Preview in Tectonic projects

AucTeX expects the output PDF after compiling to be in the same directory as the
input file. So it will error out when that is not the case, since Tectonic
places the output in a build directory.

This behavior can be controlled by using the `TeX-output-dir` variable on a per
project basis.

This configuration assumes you are using `project.el`, although porting this
code to `projectile.el` should be trivial.

```lisp
(add-hook 'after-change-major-mode-hook
          (lambda ()
            (when-let ((project (project-current))
                       (proot (project-root project)))
              (when (file-exists-p (expand-file-name "Tectonic.toml" proot))
                (setq-local TeX-output-dir (expand-file-name "build/index" proot))))))
```

We are basically looking for `Tectonic.toml` file in the project root, and if it
exists, setting the `TeX-output-dir` to the appropriate path to the build
directory. You may replace the `"build/index"` path to wherever your PDF file is
placed after it is generated by Tectonic.