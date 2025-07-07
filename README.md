# stuff

Command-line dotfiles management

## About

stuff is a tool intended to make it simple to manage dotfiles using a
repository with version control. Like some alternatives - stow or chezmoi -
it's based on linking but those tools are incomplete or over complicated.

## Organization

stuff should be run in the root directory of your project, whose relative
directory structure should mirror that of your system. Consider that the
project root maps to the root directory of the system. Then by example, for the
user `bradcush`, `project/home/bradcush/.bashrc` would automatically map to
`/home/bradcush/.bashrc` for any project scoped stuff command. Given the
commands `link`, `list`, and `unlink`, this already provides enough
functionality to easily add, view, and remove links for dotfiles.

We recommend linking both files and directories as necessary. If a system
directory should only contain links sourced from your project, it's best to
link the entire directory. Conversely, when system directories contain a
combination of links and concrete files, you should only link files
individually as to not overwrite anything on the system.

## Building

Building from source only:

`make` or `make stuff`

stuff binaries are placed in `./usr/local/bin`. We can link stuff using
itself via `sudo ./usr/local/bin/stuff link ./usr/local/bin/stuff`. `sudo` is
necessary here since root is required for changes to `/usr/local/bin`. You can
then check it's been linked correctly by calling `stuff list`. This call only
works if the binary is linked, showing in green the linked entry.

## Running

See `stuff --help` for commands and options

Example using the `list` subcommand:

``` sh
# Listing the linked binary
$ stuff list --linked
/usr/local/bin/stuff
```

### sudo

Links might need to be created in directories that only the root user has
access to for certain systems. You might need to run `sudo stuff ...` if you
see errors due to "Permission denied".

### Restrictions

- Symlinking protected directories requires `sudo`
- Commands must be run in the project root
- Unlinking requires a valid existing link
- Support for mapping is implemented

## Documentation

I've tried to make things as intuitive as I can with sane defaults. All
documentation related to usage is included in the tool itself which can be
accessed through the `--help` flag corresponding to optional commands. Specific
use cases or more detailed recommendations are covered in the Markdown.

## Support

The goal is that stuff will be available on most systems but for now I've only
developed and tested locally using Linux and specifically Arch Linux. It may or
may not work on other distributions of Linux for now.

## C language

This project is meant to be written mostly in C, a good choice for systems
programming over many decades. While other, newer languages are suitable like
Zig or Rust, I've chosen C for the simple fact that it's fast and I just want
to experience writing a robust command-line tool with it.

## Architecture

Parsing options and commands is done in a hierarchical, two-step manner where
global (hidden) options are parsed first and then command options. This gives
flexibility but also requires care in how the two may affect one another. Keep
in mind that top-level hidden options are meant for internal use only.

## Linking

Linking is an implementation detail but it can be useful to know that stuff
uses symbolic links to create links between a source in the project source and
the system. Symlinks clearly show references, can link across different
filesystems, and are allowed for directories. All of this makes them more
powerful and easier to manage for our use cases.

## Quirks

Option parsing using `getopt` has quirks when used in a hierarchical manner
which we code around for now. Some global options may interact with some
command options and vice versa.

If command options take values then global options need to be aware to avoid
unwanted behaviour. If we specify a global option then all command options need
to also be aware to avoid throwing. These cases are expected to be few in
number so we live with some sharp edges for the time being to support them
until we find a better way.

We also parse commands using `optind` which means they don't have to directly
follow the program name when specified. We consider this behaviour a feature
and not a bug for the time being.

## Stateless

The current design approach is stateless one, meaning that stuff keeps track of
no persistent state in order to function properly. The only thing stuff relies
on is an accurate local project structure, which is uses as a guide to add,
view, and remove links in a given system directory.

## Testing

All tooling related to testing is custom, specific to this project. The current
approach uses e2e (end-to-end) tests driven by a `./tests/run.sh` bash script.
Adding tests is relatively self explanatory given current examples. Since
implementation details for the tooling lives next to the tests themselves, some
knowledge of the tooling implementation might be needed to make changes.

Running e2e tests:

``` sh
# The tests script expects to be
# run in the project folder
cd ./tests/project
../run.sh

# Or more simply from the main
# stuff repository directory
make test
```

### Strategy

We think e2e tests make the most sense to start with because stuff outputs
something to `stdout` for any successful calls. We only test happy paths for
now which we think should be enough. All output is checked using `diff` against
expected output in the `./tests/output` directory. We note that calls to stuff
create a lot of side effects (eg. system calls), which we also test by checking
the intended result of those side effects.

## License

[stuff MIT License](LICENSE)
