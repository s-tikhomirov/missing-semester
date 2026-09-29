# Local llm CLI

Short guide to [simonw/llm](https://github.com/simonw/llm) with Ollama on this machine, plus the `ask` and `cmd` templates.
This is not a course lecture.

## Setup

`llm` is installed as a user-wide command (`uv tool install llm`).
The binary is on `PATH` (`~/.local/bin/llm`).
Python deps live in an isolated env.
Plugins go in that same env (`llm install llm-ollama`).

Ollama provides the weights.
The systemd unit `ollama` is enabled, so it starts at boot.
Default model for `llm` with no `-m`:

```bash
llm models default
# qwen2.5-coder:7b
```

Override one run with `-m qwen3.5:4b` (add `--think=false` on Qwen3.5 and Gemma 4 when using `ollama run`; for `llm`, pass the option if the plugin exposes it).

## Templates

Templates are YAML files under `$(llm templates path)`.
The `system:` block is the standing instruction.
Do not pin `model:` in YAML so both templates follow `llm models default`.
`-m` on the command line still wins.

`llm` interpolates `$name` in the template as a parameter.
A real dollar in the template must be written `$$`.
That is why `cmd` stores `$$()` and the model sees `$()`.

List and inspect:

```bash
llm templates list
llm templates show ask
llm templates show cmd
```

### ask

Explanations, debugging, concepts, reviewing a file on stdin.
Piped stdin is the subject to explain, not text to reprint in full.

```bash
llm -t ask 'What is SIGPIPE in a bash pipeline?'
head -n 40 ~/.bashrc | llm -t ask -s 'Explain the aliases only'
llm -t ask -m qwen3:8b 'What does export do?'
```

### cmd

Paste-ready command only.
Stdin is sample data or context.
The model writes a command that would process that kind of input.

```bash
llm -t cmd '5 most common file extensions under $HOME using find, sort, uniq -c, head'
printf '%s\n' 'foo.py' 'bar.py' 'notes.md' | llm -t cmd -s 'Count how often each extension appears'
```

Extra `-s` is the task for this run.
It stacks on the template system prompt.

## Pipes

`llm` reads stdin as the user message.
`-s` is `--system` for a one-shot instruction when you are not using `-t`, or as an extra system line with `-t`.

```bash
cat script.sh | llm -t ask
man find | col -bx | llm -t cmd -s 'A pipeline using find and uniq -c'
```

## Paste safety

Do not wrap commands in backticks.
In bash, backticks are command substitution.
The templates tell the model to put each command on its own line and to use `$()` for substitution.
If the model still emits markdown fences, run the prompt again.

## Logs

Prompts are stored in SQLite unless you turn that off.

```bash
llm logs on
llm logs list -n 5
```
