The base for this skin is http://style.tigris.org/

Got style-1.1.zip from http://style.tigris.org/servlets/ProjectDocumentList
Which provides:
images/*.gif ... we added a few of our own icons (pdfdoc, printer, xmldoc).
css/*.css ... we added forrest.css.xslt and quirks.css (not sure where
the quirks.css came from - it is not in the tigris dist).

We made some changes to the CSS (evidently these should go into
the inst.css - but not sure how) ...

tigris.css
 added: img { border: 0; }
 changed: for each image url(), removed the "../"
 changed: "inline-block" to "inline" at line 709 (fixes CSS validation error)
 added: .pdflink .xmllink .podlink .printlink
 added: .txtlink
 added: .footerLogos

