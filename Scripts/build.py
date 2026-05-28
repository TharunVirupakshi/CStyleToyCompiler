#!/usr/bin/env python3

import subprocess
import sys


def run(command):
    print(f"\n>> {' '.join(command)}")
    result = subprocess.run(command)

    if result.returncode != 0:
        print(f"\nBuild failed while running: {' '.join(command)}")
        sys.exit(result.returncode)


def main():
    # Generate lexer
    run(["lex", "lexer.l"])

    # Generate parser
    run(["bison", "-dv", "parser.y"])

    # Run custom parser log script
    run(["python3", "Scripts/add_logs_to_parser.py"])

    # Compile everything
    run([
        "gcc",
        "logger.c",
        "symTable.c",
        "ast.c",
        "semantic.c",
        "icg.c",
        "lex.yy.c",
        "parser.tab.c",
        "-o",
        "compiler",
        "-ll",
        "-ly"
    ])

    print("\nBuild successful!")
    print("Executable: ./compiler")


if __name__ == "__main__":
    main()