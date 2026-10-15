# Entry point for building the test suite. Build with: qmake portabase-tests.pro && make
#
# - tests/dbeditor: Edit Columns behavior on valid files
# - tests/dbeditor_robustness: resilience to inconsistent/corrupt file states
#   (crash cases run subprocess-isolated so a regression fails cleanly)
# - tests/integritycheck: the file-integrity checker (the `portabase check` CLI engine)
# - tests/rowlifecycle: adding and deleting data rows, and row ID assignment

TEMPLATE = subdirs
SUBDIRS = tests/dbeditor/dbeditor_test.pro \
          tests/dbeditor_robustness/dbeditor_robustness_test.pro \
          tests/integritycheck/integritycheck_test.pro \
          tests/rowlifecycle/rowlifecycle_test.pro
