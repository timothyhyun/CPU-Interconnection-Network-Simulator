#!/bin/sh

/usr/bin/time ./cadss-engine -s exp_branch.config -t /afs/cs.cmu.edu/academic/class/15346-f22/public/traces/branch/black-test.trace -b branch_impl -c refCache

/usr/bin/time ./cadss-engine -s exp_branch.config -t /afs/cs.cmu.edu/academic/class/15346-f22/public/traces/branch/cadss.trace -b branch_impl -c refCache

/usr/bin/time ./cadss-engine -s exp_branch.config -t /afs/cs.cmu.edu/academic/class/15346-f22/public/traces/branch/fluid-test.trace -b branch_impl -c refCache
