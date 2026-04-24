A few solid lessons learned from this POC:

“PDF generation” is not one problem. chromedp, maroto, and pdfcpu solve different layers: browser rendering, document layout, and low-level PDF construction/manipulation.
The best library depends more on document source than on output format. If the source is HTML, chromedp feels natural. If the source is backend data, maroto is usually cleaner. If the need is PDF internals, pdfcpu fits.
Server deployment matters as much as code quality. chromedp gives the nicest visual fidelity, but it brings Chrome, higher memory usage, startup overhead, and more ops work.
Maintainability quickly becomes the deciding factor. The pdfcpu approach in this repo works, but manual coordinates, wrapping, and encoding logic are much harder to evolve safely.
Local asset handling is important for production. The chromedp badge currently defaults to an external QR URL, which is risky on servers; local generation like the maroto and pdfcpu badge flows is safer.
Fonts and text encoding are easy to underestimate. maroto embedding fonts is a nice deployment-friendly pattern; low-level PDF generation makes Unicode and layout much easier to get wrong.
Structured business PDFs are usually better served by Go-native layout libraries than by browser rendering. For invoices, badges, and agreements, maroto is the most balanced option here.
A mixed strategy is often best. One engine for default transactional PDFs, another for high-fidelity HTML templates, and a low-level tool for specialized PDF operations tends to work better than forcing one tool for everything.
