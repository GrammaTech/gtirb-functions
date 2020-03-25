GTIRB-Functions
===============

Function objects over GTIRB

## Abstract
This repository provides simple function objects easing working with
functions on top of GTIRB instances.  Functions are not first-class in
GTIRB and are not supported by the GTIRB API.  Instead three
sanctioned AuxData tables are used to persist hold function
information in GTIRB instances.  This repository mediates access to
these tables through first-class function objects.
