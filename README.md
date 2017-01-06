# PySVN
Barry Scott's PySVN Subversion wrapper for Python, based off of the 1.9.3 version release. Official homepage for the project is http://pysvn.tigris.org/.

# Changelog
| Date | Change |
| ---- | ------ |
| 2017.01.07 | Fixed problem with `Client.diff()` throwing Unicode translation errors when there are "ASCII" characters with values in the range of 128-255 in the returned diff. |
