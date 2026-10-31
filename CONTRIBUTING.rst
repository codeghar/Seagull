How to Contribute
=================

Project status
--------------

This repository modernizes the Seagull traffic simulator (C++23, CMake,
OpenSSL 3, native Linux SCTP) and adds Diameter Gx support. Active
development happens on the ``modernizacion`` branch; ``master`` keeps the
previous state until the merge.

The original code comes from the Seagull SVN repository (see *Origin* in
``README.md``). Thanks to Hamza Sheikh (@aikchar), previous maintainer of
this repository, for the patches that made Seagull build on modern Linux.

Reporting issues
----------------

Open a GitHub issue with:

* Commit (``git log --oneline -1``)
* OS, compiler and OpenSSL version
* Transport (TCP, TLS or SCTP), configuration and scenario files
* Command line, ``client.out`` / ``server.out`` and the relevant log lines
* For network problems, a packet capture (``tcpdump``) if possible

Do not attach private keys, credentials or production data.

Submitting changes
------------------

1. Branch from ``modernizacion``.
2. Build and run the full validation before opening a pull request::

       cd seagull/trunk/src
       tests/validate-all.sh

   It must end with ``VALIDACION COMPLETA: TODO OK``. If the change affects
   a transport, also test it between two machines when possible.
3. Keep one logical change per commit. Commit messages follow this format::

       fix(scope): short summary

       Root cause.
       What changed.
       How it was validated.

   Use ``feat``, ``fix``, ``docs`` or ``test`` as type.
4. Reproduce a bug before fixing it, and include the before/after results
   in the commit message.

License
-------

Contributions are accepted under the GNU General Public License v2 or
later, the same license as the original code. Keep the existing copyright
headers.

Contact
-------

yeimijh@tutanota.com
