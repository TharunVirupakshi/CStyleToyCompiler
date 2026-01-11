import re
import json
import os

GRAMMAR_START = "// GRAMMAR RULES START"
GRAMMAR_END = "// GRAMMAR RULES END"

OUTPUT_DIR = "Grammar"
OUTPUT_FILE = "grammar.json"

log_rule_re = re.compile(
    r'log_rule\(\s*"([^"]+)"\s*,\s*(\d+)\s*\)'
)

log_semantic_re = re.compile(
    r'log_semantic_step\(\s*"([^"]+)"\s*,\s*(\d+)\s*,\s*(\d+)\s*\)'
)


def extract_grammar_rules(parser_y_path):
    with open(parser_y_path, "r") as f:
        lines = f.readlines()

    inside = False
    grammar = {}

    for line in lines:
        if GRAMMAR_START in line:
            inside = True
            continue
        if GRAMMAR_END in line:
            break
        if not inside:
            continue

        rule_match = log_rule_re.search(line)
        if rule_match:
            text, rule_no = rule_match.groups()
            rule_no = int(rule_no)

            grammar.setdefault(rule_no, {
                "ruleNo": rule_no,
                "text": text,
                "semanticSteps": []
            })

        sem_match = log_semantic_re.search(line)
        if sem_match:
            instr, rule_no, step_no = sem_match.groups()
            rule_no = int(rule_no)
            step_no = int(step_no)

            grammar.setdefault(rule_no, {
                "ruleNo": rule_no,
                "text": "",
                "semanticSteps": []
            })

            grammar[rule_no]["semanticSteps"].append({
                "stepNo": step_no,
                "instr": instr
            })

    rules = list(grammar.values())
    rules.sort(key=lambda r: r["ruleNo"])

    for r in rules:
        r["semanticSteps"].sort(key=lambda s: s["stepNo"])

    return rules


if __name__ == "__main__":
    rules = extract_grammar_rules("parser.y")

    # ✅ Create Grammar folder if not exists
    os.makedirs(OUTPUT_DIR, exist_ok=True)

    output_path = os.path.join(OUTPUT_DIR, OUTPUT_FILE)
    with open(output_path, "w") as f:
        json.dump(rules, f, indent=2)

    print(f"Extracted {len(rules)} grammar rules → {output_path}")
