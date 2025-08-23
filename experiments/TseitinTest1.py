#! /usr/bin/env python

"""
This script uses the FastDownwardExperiment class, which makes the script very
concise, but harder to extend once you go beyond "standard" experiments.
Therefore, we recommend using the Experiment class directly. See the script in
examples/downward/2020-09-11-B-bounded-cost.py for an example.
"""

import os
from downward.reports.scatter import ScatterPlotReport
import custom_parser
import project

REPO = os.path.expanduser("~/Downward/downward")
BENCHMARKS_DIR = os.environ["DOWNWARD_BENCHMARKS"]
SCP_LOGIN = "jan.walliser@unibas.ch"
REMOTE_REPOS_DIR = "/infai/username/projects"
# If REVISION_CACHE is None, the default "./data/revision-cache/" is used.
REVISION_CACHE = os.environ.get("DOWNWARD_REVISION_CACHE")
DOMAINS = [
    "sokoban-axioms",
    "psr-middle-noce",
    "psr-middle",
    "optical-telegraphs",
    "philosophers",
    "acc-cc2-ghosh-etal",
    "doorexample-broken-ghosh-etal",
    "doorexample-fixed-ghosh-etal",
    "grid-axioms",
   # "grid-cc2-ghosh-etal", has undeclared object
    "miconic-axioms",
   # "mincut", uses object fluent not supported by the planner
    "trapping_game",
    "collab-and-comm-kg",
    "muddy-child-kg",
    "muddy-children-kg",
    "sum-kg",
    "wordrooms-kg",
]

if project.REMOTE:
    SUITE = DOMAINS #project.SUITE_SATISFICING
    ENV = project.BaselSlurmEnvironment(partition="infai_2", email="jan.walliser@unibas.ch")
else:
    SUITE = DOMAINS
    #    SUITE = project.SUITE_SATISFICING         
    ENV   = project.LocalEnvironment(processes=8)
      
CONFIGS = [
    # Direkt mit Suchausdruck
    ("ff-tseitin-negative-no-search",
     ["--search", "eager_greedy([ff(axioms=approximate_negative,transform=tseitin())])"],
     []),

    ("ff-tseitin-negative-cycles-no-search",
     ["--search", "eager_greedy([ff(axioms=approximate_negative_cycles,transform=tseitin())])"],
     []),

    # Alias-Varianten -> gehören in die driver options
    ("ff-tseitin-negative",
     ["--search","eager_greedy([ff(axioms=approximate_negative,transform=tseitin())],transform=tseitin())"],
     []),

    ("ff-tseitin-negative-cycles",
     ["--search","eager_greedy([ff(axioms=approximate_negative_cycles,transform=tseitin())],transform=tseitin())"],
     []),

    # Baselines ohne Transform
    ("ff-plain-negative",
     ["--search", "eager_greedy([ff(axioms=approximate_negative)],verbosity=normal)"],
     []),

    ("ff-plain-negative-cycles",
     ["--search", "eager_greedy([ff(axioms=approximate_negative_cycles)],verbosity=normal)"],
     []),
         # Direkt mit Suchausdruck
    ("add-tseitin-negative-no-search",
     ["--search", "eager_greedy([add(axioms=approximate_negative,transform=tseitin())])"],
     []),

    ("add-tseitin-negative-cycles-no-search",
     ["--search", "eager_greedy([add(axioms=approximate_negative_cycles,transform=tseitin())])"],
     []),

    # Alias-Varianten -> gehören in die driver options
    ("add-tseitin-negative",
     ["--search", "eager_greedy([add(axioms=approximate_negative,transform=tseitin())],transform=tseitin())"],
     []),

    ("add-tseitin-negative-cycles",
     ["--search", "eager_greedy([add(axioms=approximate_negative_cycles,transform=tseitin())],transform=tseitin())"],
     []),

    # Baselines ohne Transform
    ("add-plain-negative",
     ["--search", "eager_greedy([add(axioms=approximate_negative)],verbosity=normal)"],
     []),

    ("add-plain-negative-cycles",
     ["--search", "eager_greedy([add(axioms=approximate_negative_cycles)],verbosity=normal)"],
     []),
]

BUILD_OPTIONS = []
DRIVER_OPTIONS = [
    "--validate",
    "--search-time-limit", "30m",
    "--search-memory-limit", "3G",
  ]

REV_NICKS = [
    ("HEAD", ""),
]
ATTRIBUTES = [
    "error",
    "run_dir",
    "search_start_time",
    "search_start_memory",
    "total_time",
    "h_values",
    "coverage",
    "expansions",
    "memory",
   	
    project.EVALUATIONS_PER_TIME,
]

exp = project.FastDownwardExperiment(environment=ENV, revision_cache=REVISION_CACHE)
for config_nick, config, extra_options in CONFIGS:
    for rev, rev_nick in REV_NICKS:
        algo_name = f"{rev_nick}:{config_nick}" if rev_nick else config_nick
        exp.add_algorithm(
            algo_name,
            REPO,
            rev,
            config,
            build_options=BUILD_OPTIONS,
            driver_options=DRIVER_OPTIONS+extra_options,
        )
exp.add_suite(BENCHMARKS_DIR, SUITE)

exp.add_parser(exp.EXITCODE_PARSER)
exp.add_parser(exp.TRANSLATOR_PARSER)
exp.add_parser(exp.SINGLE_SEARCH_PARSER)
exp.add_parser(custom_parser.get_parser())
exp.add_parser(exp.PLANNER_PARSER)

exp.add_step("build", exp.build)
exp.add_step("start", exp.start_runs)
exp.add_step("parse", exp.parse)
exp.add_fetcher(name="fetch")

project.add_absolute_report(
    exp, attributes=ATTRIBUTES, filter=[project.add_evaluations_per_time]
)

if not project.REMOTE:
    project.add_scp_step(exp, SCP_LOGIN, REMOTE_REPOS_DIR)

# ==== SCATTERPLOTS (Plain vs. Tseitin) ====

def add_scatter_pair(exp, left, right, attr, name_suffix="", scale="log"):
    exp.add_report(
        ScatterPlotReport(
            attributes=[attr],                     # exakt 1 Attribut
            filter_algorithm=[left, right],        # exakt 2 Algorithmen
            get_category=lambda r1, r2: r1["domain"],
            show_missing=False,                    # Tasks ohne beide Runs werden gedroppt
            format="png",
            scale=scale,
            xlabel=left,
            ylabel=right,
            # optional schön: reference-Linie
            # reference="diagonal",
        ),
        name=f"{exp.name}-scatter-{attr}-{left}-vs-{right}{name_suffix}",
    )

# Suche+Heuristik: FF & Add, beide Axiom-Modi (passt bei dir bereits)
search_pairs = [
    ("ff-plain-negative",         "ff-tseitin-negative"),
    ("ff-plain-negative-cycles",  "ff-tseitin-negative-cycles"),
    ("add-plain-negative",        "add-tseitin-negative"),
    ("add-plain-negative-cycles", "add-tseitin-negative-cycles"),
]

# Heuristik-only (Transform nur in der Heuristik, nicht in der Suche)
# -> Baseline ist *plain* (ohne Transform), Vergleich ist *-no-search*
eval_pairs = [
    ("ff-plain-negative",         "ff-tseitin-negative-no-search"),
    ("ff-plain-negative-cycles",  "ff-tseitin-negative-cycles-no-search"),
    ("add-plain-negative",        "add-tseitin-negative-no-search"),
    ("add-plain-negative-cycles", "add-tseitin-negative-cycles-no-search"),
]

# Plots: memory (log) und initial_h_value (linear)
for l, r in search_pairs:
    add_scatter_pair(exp, l, r, "memory",          scale="log")
    add_scatter_pair(exp, l, r, "initial_h_value", scale="linear")
    add_scatter_pair(exp, l, r, "search_time",	   scale="log")
for l, r in eval_pairs:
    add_scatter_pair(exp, l, r, "memory",          name_suffix="-heuronly", scale="log")
    add_scatter_pair(exp, l, r, "initial_h_value", name_suffix="-heuronly", scale="linear")
    add_scatter_pair(exp, l, r, "search_time",	   name_suffix="-heuronly", scale="log")
project.add_compress_exp_dir_step(exp)

exp.run_steps()

