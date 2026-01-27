#!/usr/bin/env python3
import os
import re
import xml.etree.ElementTree as ET

JUNIT_REPORT = os.environ.get("JUNIT_REPORT", "")
TEST_LOG = os.environ.get("TEST_LOG", "")
GITHUB_OUTPUT = os.environ.get("GITHUB_OUTPUT", "")


def parse_junit(path: str) -> dict:
    results = {"total": 0, "failed": 0, "failures": []}

    def handle_suite(suite):
        for case in suite.findall("testcase"):
            results["total"] += 1
            failed = case.find("failure") is not None or case.find("error") is not None
            if failed:
                results["failed"] += 1
                name = case.get("name", "") or "unknown"
                classname = case.get("classname", "")
                label = f"{classname}::{name}" if classname else name
                results["failures"].append(label)
        for child in suite.findall("testsuite"):
            handle_suite(child)

    tree = ET.parse(path)
    root = tree.getroot()
    if root.tag == "testsuite":
        handle_suite(root)
    else:
        for suite in root.findall("testsuite"):
            handle_suite(suite)

    return results


def parse_ctest_log(path: str) -> dict:
    results = {"total": 0, "failed": 0, "failures": []}
    summary_pattern = re.compile(
        r"^\s*\d+/\d+\s+Test #\d+:\s+(?P<name>.+?)\s+\.+\s*\*{0,3}(?P<status>Passed|Failed)\b"
    )
    start_pattern = re.compile(r"^\s*Start\s+\d+:\s+(?P<name>.+?)\s*$")
    failed_list_header = re.compile(r"^\s*The following tests FAILED:", re.IGNORECASE)
    failed_list_entry = re.compile(r"^\s*\d+\s*-\s*(?P<name>.+?)\s+\(Failed\)\s*$")
    qt_totals_pattern = re.compile(
        r"^\s*Totals:\s+(?P<passed>\d+)\s+passed,\s+(?P<failed>\d+)\s+failed",
        re.IGNORECASE,
    )
    qt_failure_pattern = re.compile(r"^\s*FAIL!\s*:\s+(?P<name>.+?)\s")

    start_names = []
    failures = []
    has_summary = False
    in_failed_list = False
    qt_totals_found = False
    qt_passed = 0
    qt_failed = 0
    qt_failures = []

    with open(path, "r", encoding="utf-8", errors="ignore") as handle:
        for line in handle:
            qt_totals_match = qt_totals_pattern.search(line)
            if qt_totals_match:
                qt_totals_found = True
                qt_passed += int(qt_totals_match.group("passed"))
                qt_failed += int(qt_totals_match.group("failed"))

            qt_failure_match = qt_failure_pattern.search(line)
            if qt_failure_match:
                qt_failures.append(qt_failure_match.group("name").strip())

            if failed_list_header.search(line):
                in_failed_list = True
                continue

            if in_failed_list:
                entry_match = failed_list_entry.search(line)
                if entry_match:
                    failures.append(entry_match.group("name").strip())
                    continue
                if line.strip() == "":
                    in_failed_list = False

            summary_match = summary_pattern.search(line)
            if summary_match:
                has_summary = True
                results["total"] += 1
                status = summary_match.group("status")
                name = summary_match.group("name").strip()
                if status == "Failed":
                    results["failed"] += 1
                    results["failures"].append(name)
                continue

            start_match = start_pattern.search(line)
            if start_match:
                start_names.append(start_match.group("name").strip())

    if qt_totals_found:
        results["total"] = qt_passed + qt_failed
        results["failed"] = qt_failed
        results["failures"] = list(dict.fromkeys(qt_failures))
        return results

    if not has_summary and start_names:
        results["total"] = len(start_names)
        results["failed"] = len(failures)
        results["failures"] = failures

    return results


results = {"total": 0, "failed": 0, "failures": []}

if JUNIT_REPORT and os.path.exists(JUNIT_REPORT):
    try:
        results = parse_junit(JUNIT_REPORT)
    except ET.ParseError:
        results = {"total": 0, "failed": 0, "failures": []}

if TEST_LOG and os.path.exists(TEST_LOG):
    ctest_results = parse_ctest_log(TEST_LOG)
    if (
        ctest_results["total"] > results["total"]
        or (ctest_results["failed"] > results["failed"])
    ):
        results = ctest_results

passed = max(results["total"] - results["failed"], 0)

if GITHUB_OUTPUT:
    with open(GITHUB_OUTPUT, "a", encoding="utf-8") as output_file:
        output_file.write(f"tests_total={results['total']}\n")
        output_file.write(f"tests_passed={passed}\n")
        output_file.write(f"tests_failed={results['failed']}\n")
        output_file.write("failed_tests<<EOF\n")
        output_file.write("\n".join(results["failures"]))
        output_file.write("\nEOF\n")
