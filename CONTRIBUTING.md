# Style

This project is formatted with [clang-format][fmt] using the style file at the root of the repository. Please run clang-format before sending a pull request.

In general, try to follow the style of surrounding code. We mostly follow the [Google C++ style guide][cpp-style].

Commit messages should be in the imperative mood, as described in the [Git contributing file][git-contrib]:

> Describe your changes in imperative mood, e.g. "make xyzzy do frotz"
> instead of "[This patch] makes xyzzy do frotz" or "[I] changed xyzzy
> to do frotz", as if you are giving orders to the codebase to change
> its behaviour.

[fmt]: http://clang.llvm.org/docs/ClangFormat.html
[cpp-style]: https://google.github.io/styleguide/cppguide.html
[git-contrib]: http://git.kernel.org/cgit/git/git.git/tree/Documentation/SubmittingPatches?id=HEAD

# Tests

Please verify the tests pass by configuring CMake with `-D YAML_CPP_BUILD_TESTS=ON` and running the target `test/yaml-cpp-tests`.

If you are adding functionality, add tests accordingly. Note that the "spec tests" are reserved for examples directly from the YAML spec, so if you have new examples, put them in other test files.

# Pull request process

Every pull request undergoes a code review. Unfortunately, github's code review process isn't great, but we'll manage. During the code review, if you make changes, add new commits to the pull request for each change. Once the code review is complete, rebase against the master branch and squash into a single commit.

# AI Usage

The following rules are in service of producing correct, maintainable code while not inundating the maintainers.

1. You (the author of the PR) are responsible for the PR, regardless of the tools you use to create it. Make sure you understand it thoroughly and agree with its contents before you submit a PR.
2. Corollary: AI is not a co-author, AI is a tool. If you wouldn't list Emacs as a co-author, don't list Claude either. (Maybe it will be someday, but not yet.)
3. Do not use AI to generate documentation. It's fine for code, but it's [painful](https://samkriss.substack.com/p/if-you-let-ai-do-your-writing-i-will) to read AI text.
4. No "low effort" AI PRs. Don't just paste an issue into Claude and ask it to fix it; don't just ask it for a vulnerability report. That's fine as a starting point, but if that's *all* you do, then you haven't provided any value; you've just added work for the maintainers, who could have easily pasted the issue into Claude themselves.
