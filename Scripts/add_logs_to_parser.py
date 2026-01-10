import os
import re
import json

STATE_RE = re.compile(r"^State (\d+)")
ITEM_RE = re.compile(r"^\s*(\d+)\s+(.*)")
SHIFT_RE = re.compile(r"^\s*(\S+)\s+shift, and go to state (\d+)")
REDUCE_LOOKAHEAD_RE = re.compile(
    r"^\s*(\S+)\s+\[reduce using rule (\d+)\s+\((.+)\)\]"
)
DEFAULT_REDUCE_RE = re.compile(
    r"^\s*\$default\s+reduce using rule (\d+)\s+\((.+)\)"
)
DEFAULT_ACCEPT_RE = re.compile(r"^\s*\$default\s+accept")

def parse_parser_output(text):
    states = []
    current = None
    section = "items"  # items → shifts → reduces

    for line in text.splitlines():
        # New state
        m = STATE_RE.match(line)
        if m:
            if current:
                states.append(current)
            current = {
                "state": int(m.group(1)),
                "items": [],
                "shifts": [],
                "reduces": [],
                "default": None
            }
            section = "items"
            continue

        if not current:
            continue

        # Blank line moves section forward
        if line.strip() == "":
            if section == "items":
                section = "shifts"
            elif section == "shifts":
                section = "reduces"
            continue

        # ---- ITEMS ----
        if section == "items":
            m = ITEM_RE.match(line)
            if m:
                rule, item = m.groups()
                current["items"].append({
                    "rule": int(rule),
                    "item": item.strip()
                })
            continue

        # ---- SHIFTS ----
        m = SHIFT_RE.match(line)
        if m:
            sym, to = m.groups()
            current["shifts"].append({
                "symbol": sym,
                "to": int(to)
            })
            continue

        # ---- LOOKAHEAD REDUCE ----
        m = REDUCE_LOOKAHEAD_RE.match(line)
        if m:
            sym, rule, lhs = m.groups()
            current["reduces"].append({
                "symbol": sym,
                "rule": int(rule),
                "lhs": lhs
            })
            continue

        # ---- DEFAULT REDUCE ----
        m = DEFAULT_REDUCE_RE.match(line)
        if m:
            rule, lhs = m.groups()
            current["default"] = {
                "type": "reduce",
                "rule": int(rule),
                "lhs": lhs
            }
            continue

        # ---- DEFAULT ACCEPT ----
        if DEFAULT_ACCEPT_RE.match(line):
            current["default"] = {
                "type": "accept"
            }
            continue

    if current:
        states.append(current)

    return states


def main():
    input_file = "parser.output"
    output_dir = "States"
    output_file = os.path.join(output_dir, "states.json")

    os.makedirs(output_dir, exist_ok=True)

    with open(input_file, "r") as f:
        text = f.read()

    states = parse_parser_output(text)

    with open(output_file, "w") as f:
        json.dump(states, f, indent=2)

    print(f"✔ Parsed {len(states)} states")
    print(f"✔ Output written to {output_file}")


if __name__ == "__main__":
    main()
