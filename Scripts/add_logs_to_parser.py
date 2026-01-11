import re

with open("parser.tab.c") as f:
    text = f.read()

pattern = re.compile(
    r"# define YY_STACK_PRINT\(Bottom, Top\)[\s\S]*?while \(0\)",
    re.MULTILINE
)

replacement = """# define YY_STACK_PRINT(Bottom, Top)                            \\
do {                                                            \\
  if (yydebug) {                                                \\
    yy_stack_print ((Bottom), (Top));                           \\
    log_parse_stack_snapshot((Bottom), (Top));                  \\
  }                                                             \\
} while (0)"""

text, n = pattern.subn(replacement, text)

if n != 1:
    raise RuntimeError(f"Expected 1 replacement, got {n}")

with open("parser.tab.c", "w") as f:
    f.write(text)
