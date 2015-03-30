# Introduction #

Understandably, the yaml-cpp management would like to keep the /trunk in a fairly reasonable state for consumers of the yaml-cpp library.  There are, however, occasions that arise where multiple patch branches that do not merge cleanly need to be integrated before they can be combined back into /trunk.  In these cases we fall back on staging branches.


# Where Are the Staging Branches? #

Staging branches, like other branches, are found in the project's Subversion repository under the /branches folder.  Anonymous access is available at
http://yaml-cpp.googlecode.com/svn/branches.  The staging branches have the form _username_-staging.

As of this writing, the only staging branch is http://yaml-cpp.googlecode.com/svn/branches/rtweeks21-staging, but please check the branches link (from the previous paragraph) for the latest information.


# What is in a Staging Branch? #

That is really up to the developer who owns the branch.  Staging branches will typically look like a copy of the HEAD of /trunk into which some other branches have been reintegrated.  Staging branches are intended to provide an area to prepare a group of (possibly unrelated) changes to go into the trunk.


# How Will the Developers Keep Track of the Merges? #

All Subversion clients starting with version 1.5 add information about merges between branches to appropriate folders in the svn:mergeinfo property.  These clients can therefore track (and report) which version have been merged from one branch to another and avoid any attempts to re-merge a revision.

The best way to merge a feature or bug-fix branch into a staging branch (or a staging branch into /trunk) is `svn merge --reintegrate`, which will correctly handle changes from any synchronization merges from /trunk into the development branch.  This follows the procedure recommended by the Subversion documentation.