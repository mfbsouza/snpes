# How to contribute

First of all, thanks for the interest in contributing to the project.

Contributions can come in many forms like:

- Feature requests
- Issue discussions
- Documentation
- Reporting bugs of spelling errors

If you want to contribute with code than you can check the following sections
about the project's git structure and general rules.

## Sections

- [Public branches](#public-branches)
- [General rules around public branches](#general-rules-around-public-branches)
- [General rules around commits](#general-rules-around-commits)
- [Outside contributions](#outside-contributions)
- [Sugested workflow](#sugested-workflow)

## Public branches

There are 2 public branches: `main` and `dev`.

**main**: represents the project in a production state.
where any normal user can download and use it.

**dev**: represents the project in a development state.
it's targeted to inside developers and outside contributers.

## General rules around public branches

1. never rebase a public branch
2. merge commits only happens when sending code from dev to main
and when sending hotfixes to dev and to main
3. always do a merge fast-forward when adding your changes to the dev branch
4. changes to the dev branch must be in the form of atomic commits

## General rules around commits

1. every commit on public branches must be signed-off
2. gpg signed commits are optional

## Outside contributions

contributions from developers that doesn't have write permissions to
the repository must come from `patches` and/or `pull requests`.

### General rules around patches

1. patches must be created with git format-patch
2. patches must be signed-off
3. patches must be atomic changes

### Sugested workflow for patches

1. create you working branch:
    ```
    $ git checkout -b it-really-doesnt-matter-the-name
    ```
2. lots of working:
    ```
    # some amount of work
    $ git add <your changes>
    $ git commit
    
    # another amount of work
    $ git add <your changes>
    $ git commit

    # and it really doesnt matter your commit msgs in this phase
    ```
3. squashing your changes into one atomic commit:
    ```
    # go back to the dev branch and update it
    $ git checkout dev
    $ git pull

    # go back to your working branch and do a squash and rebase
    $ git checkout <my-working-branch-name>
    $ git rebase -i --signoff dev
    # now squash all your commits into one and write a atomic commit msg
    ```
4. creating the patch:
    ```
    # at you working branch do
    $ git format-patch -1
    ```

### General rules around pull requests

1. pull request must be targeted to the dev branch
2. pull request must be signed-off
2. pull request must be atomic changes

## Sugested workflow

Assuming you're already in the dev branch and you're about to start working

1. create you working branch:
    ```
    $ git checkout -b it-really-doesnt-matter-the-name
    ```
2. lots of working:
    ```
    # some amount of work
    $ git add <your changes>
    $ git commit
    
    # another amount of work
    $ git add <your changes>
    $ git commit

    # and it really doesnt matter your commit msgs in this phase
    ```
3. sending your changes to your local dev branch:
    ```
    # go back to the dev branch and update it
    $ git checkout dev
    $ git pull

    # go back to your working branch and do a squash and rebase
    $ git checkout <my-working-branch-name>
    $ git rebase -i --signoff dev
    # now squash all your commits into one and write a atomic commit msg

    # now merging to the dev branch...
    $ git checkout dev
    $ git merge --ff-only <my-working-branch-name>
    ```
4. sending you changes to the remote dev branch:
    ```
    # hey, someone may be faster than you at sending commits.
    # check again if there is any new commits, from the dev branch do:
    $ git fetch

    # if there are new commits from the remote do:
    $ git pull --rebase
    
    # now, send it to the remote dev branch
    $ git push
    ```

