import re
import json
import os
import sys

STATE_RE = re.compile(r"^State\s+(\d+)\s*$")
ITEM_RE = re.compile(r"^\s*(\d+)\s+(.*)$")
SHIFT_RE = re.compile(r"^\s*(\S+)\s+shift, and go to state\s+(\d+)")
REDUCE_RE = re.compile(r"^\s*(\S+)\s+\[reduce using rule\s+(\d+)\s+\(([^)]+)\)\]")
DEFAULT_REDUCE_RE = re.compile(r"^\s*\$default\s+reduce using rule\s+(\d+)\s+\(([^)]+)\)")
DEFAULT_ACCEPT_RE = re.compile(r"^\s*\$default\s+accept")
GOTO_RE = re.compile(r"^\s*(\S+)\s+go to state\s+(\d+)")
END_SHIFT_RE = re.compile(r"^\s*\$end\s+shift, and go to state\s+(\d+)")

def parse_parser_output(text):
    states = []
    current = None
    parsing_states = False

    for line in text.splitlines():
        # Start parsing only on real State headers
        m = STATE_RE.match(line)
        if m:
            parsing_states = True
            current = {
                "state": int(m.group(1)),
                "items": [],
                "shifts": [],
                "reduces": [],
                "gotos": [],
                "default": None
            }
            states.append(current)
            continue

        if not parsing_states or current is None:
            continue

        # Item
        m = ITEM_RE.match(line)
        if m:
            rule, item = m.groups()
            current["items"].append({
                "rule": int(rule),
                "item": item.strip()
            })
            continue

        # $end shift
        m = END_SHIFT_RE.match(line)
        if m:
            current["shifts"].append({
                "symbol": "$end",
                "to": int(m.group(1))
            })
            continue

        # Shift
        m = SHIFT_RE.match(line)
        if m:
            sym, state = m.groups()
            current["shifts"].append({
                "symbol": sym,
                "to": int(state)
            })
            continue

        # Reduce
        m = REDUCE_RE.match(line)
        if m:
            sym, rule, lhs = m.groups()
            current["reduces"].append({
                "symbol": sym,
                "rule": int(rule),
                "lhs": lhs
            })
            continue

        # Default reduce
        m = DEFAULT_REDUCE_RE.match(line)
        if m:
            rule, lhs = m.groups()
            current["default"] = {
                "action": "reduce",
                "rule": int(rule),
                "lhs": lhs
            }
            continue

        # Default accept
        if DEFAULT_ACCEPT_RE.match(line):
            current["default"] = {
                "action": "accept"
            }
            continue

        # Goto
        m = GOTO_RE.match(line)
        if m:
            sym, state = m.groups()
            current["gotos"].append({
                "symbol": sym,
                "to": int(state)
            })
            continue

    return states




def main():
    input_file = "parser.output"
    output_dir = "States"
    output_file = os.path.join(output_dir, "states.json")

    os.makedirs(output_dir, exist_ok=True)

    with open(input_file) as f:
        text = f.read()

    states = parse_parser_output(text)

    with open(output_file, "w") as f:
        json.dump(states, f, indent=2)

    print(f"✔ Parsed {len(states)} states")
    print(f"✔ Output written to {output_file}")


if __name__ == "__main__":
    main()
