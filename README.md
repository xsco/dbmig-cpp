dbmig
=====

dbmig is a schema management tool for relational databases that is fully compatible with Continuous Delivery. It aims to set and fulfill a gold standard in this area by providing the capability for full automation throughout the software development lifecycle.

dbmig is free software, meaning that anyone can use, share, and modify it. It is licensed under the GNU General Public License (GPL), version 3.

For more information, see the main website at http://dbmig.xsco.org

Compilation
-----------

dbmig is built using GNU Autotools.  Normally, the following commands should
be sufficient to configure and build the package:

    $ ./bootstrap.sh
    $ ./configure
    $ make

You can then install the package (with sufficient privileges) by issuing:

    # make install

Dependencies
------------

dbmig requires suitable versions of the following libraries:

* [Boost](http://www.boost.org)
* [Boost.Nowide](http://cppcms.com/files/nowide/html)
* [SOCI](http://soci.sourceforge.net)
* [Crypto++](http://www.cryptopp.com)

The `configure` script should report an error if any of the necessary
dependencies are missing.

