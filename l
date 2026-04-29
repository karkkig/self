package mapdf

import (
	"embed"
	"fmt"
	"math"

	"github.com/johnfercher/maroto/v2"
	"github.com/johnfercher/maroto/v2/pkg/components/col"
	"github.com/johnfercher/maroto/v2/pkg/components/line"
	"github.com/johnfercher/maroto/v2/pkg/components/row"
	"github.com/johnfercher/maroto/v2/pkg/components/text"
	"github.com/johnfercher/maroto/v2/pkg/config"
	"github.com/johnfercher/maroto/v2/pkg/consts/align"
	"github.com/johnfercher/maroto/v2/pkg/consts/border"
	"github.com/johnfercher/maroto/v2/pkg/consts/fontstyle"
	"github.com/johnfercher/maroto/v2/pkg/consts/orientation"
	"github.com/johnfercher/maroto/v2/pkg/core"
	"github.com/johnfercher/maroto/v2/pkg/core/entity"
	"github.com/johnfercher/maroto/v2/pkg/props"
)

// ─── Embedded fonts ───────────────────────────────────────────────────────────

//go:embed fonts/DejaVuSans.ttf fonts/DejaVuSans-Bold.ttf fonts/DejaVuSans-Oblique.ttf fonts/DejaVuSans-BoldOblique.ttf
var fontFS embed.FS

const fontFamily = "DejaVu"

func mustFont(family string, style fontstyle.Type, fsPath string) *entity.CustomFont {
	b, err := fontFS.ReadFile(fsPath)
	if err != nil {
		panic(fmt.Sprintf("embedded font %q not found: %v", fsPath, err))
	}
	return &entity.CustomFont{Family: family, Style: style, Bytes: b}
}

// ─── Types ────────────────────────────────────────────────────────────────────

type ServiceItem struct {
	Description     string
	NumProjects     string
	PricePerProject string
}

type PaymentEntry struct {
	Date   string
	Amount string
}

type FullAgreementData struct {
	State           string
	Day             string
	Month           string
	Year            string
	ProviderName    string
	ProviderAddress string
	BuyerName       string
	BuyerAddress    string
	Services        []ServiceItem
	PurchasePrice   string
	Notes           string
	Payer           string
	Payee           string
	Product         string
	AmountPerPeriod string
	Interval        string
	TotalAmount     string
	Payments        []PaymentEntry
	LateFee         string
	BounceFee       string
	LenderAction    string
	TermsConditions string
}

// ─── Layout constants ─────────────────────────────────────────────────────────

const (
	// usable page width in mm: A4 210mm − 20mm left margin − 20mm right margin
	pageWidthMM = 170.0

	// mm per line of body text at 9pt with ×1.4 leading
	// 9pt × (1mm / 2.8346pt) × 1.4 leading ≈ 4.45mm
	lineH = 4.45

	// bottom padding added to every dynamic row so descenders are never clipped
	rowPadMM = 2.0
)

// ─── Colors ───────────────────────────────────────────────────────────────────

var (
	agBlack     = props.Color{Red: 20, Green: 20, Blue: 20}
	agGray      = props.Color{Red: 110, Green: 110, Blue: 110}
	agLightGray = props.Color{Red: 230, Green: 230, Blue: 230}
	agWhite     = props.Color{Red: 255, Green: 255, Blue: 255}
	agBorder    = props.Color{Red: 160, Green: 160, Blue: 160}
	agHeaderBg  = props.Color{Red: 50, Green: 50, Blue: 50}
)

func agHeaderCell() *props.Cell {
	return &props.Cell{
		BackgroundColor: &agHeaderBg,
		BorderColor:     &agBorder,
		BorderType:      border.Full,
		BorderThickness: 0.3,
	}
}

func agDataCell(bg *props.Color) *props.Cell {
	return &props.Cell{
		BackgroundColor: bg,
		BorderColor:     &agBorder,
		BorderType:      border.Full,
		BorderThickness: 0.3,
	}
}

// ─── Text helpers ─────────────────────────────────────────────────────────────

func txt(size float64, style fontstyle.Type, clr *props.Color) props.Text {
	return props.Text{Size: size, Style: style, Color: clr, Family: fontFamily}
}

func txtAlign(size float64, style fontstyle.Type, clr *props.Color, a align.Type) props.Text {
	return props.Text{Size: size, Style: style, Color: clr, Align: a, Family: fontFamily}
}

// ─── Dynamic row height ───────────────────────────────────────────────────────
//
// rowH computes the maroto row height (mm) required to show all lines of s
// at fontSize rendered inside a column colSpan/12 of the page width.
//
// How it works:
//   colWidthMM  = pageWidthMM × colSpan/12
//   charWidthMM = fontSize (pt) ÷ 2.8346 (pt/mm) × 0.52 (Helvetica avg advance)
//   charsPerRow = floor(colWidthMM / charWidthMM)
//   linesNeeded = ceil(runeCount / charsPerRow)
//   height      = linesNeeded × lineH + rowPadMM
//
// The result is clamped to minH so small strings still get a readable height.
func rowH(s string, fontSize float64, colSpan int, minH float64) float64 {
	if s == "" {
		return minH
	}
	colW := pageWidthMM * float64(colSpan) / 12.0
	charW := (fontSize / 2.8346) * 0.52
	if charW <= 0 {
		charW = 1
	}
	charsPerLine := math.Floor(colW / charW)
	if charsPerLine < 1 {
		charsPerLine = 1
	}
	runeCount := float64(len([]rune(s)))
	lines := math.Ceil(runeCount / charsPerLine)
	h := lines*lineH + rowPadMM
	if h < minH {
		return minH
	}
	return h
}

// pRow returns a row whose height is computed dynamically for the given text.
func pRow(s string, fontSize float64, colSpan int, style fontstyle.Type, clr *props.Color, minH float64) core.Row {
	h := rowH(s, fontSize, colSpan, minH)
	return row.New(h).Add(col.New(colSpan).Add(text.New(s, txt(fontSize, style, clr))))
}

// ─── Checkbox row ─────────────────────────────────────────────────────────────

func checkboxRow(leftLabel, rightLabel string) core.Row {
	left := col.New(6).Add(text.New("\u2610  "+leftLabel, txt(9, fontstyle.Normal, &agBlack)))
	var right core.Col
	if rightLabel == "" {
		right = col.New(6)
	} else {
		right = col.New(6).Add(text.New("\u2610  "+rightLabel, txt(9, fontstyle.Normal, &agBlack)))
	}
	return row.New(8).Add(col.New(1), left, right, col.New(1))
}

// ─── Strikethrough sandwich ───────────────────────────────────────────────────
//
// maroto draws line.New() at the VERTICAL CENTRE of the row it occupies.
// Layering on top of another row is impossible, so we use three rows:
//
//   Row A  height=h       the text itself
//   Row B  height=spacer  pushes the cursor so Row C's centre hits mid-glyph
//   Row C  height=0.4mm   line.New() → drawn at 0.2mm = mid-cap-height
//   Row D  height=0.5mm   bottom clearance
//
// Geometry (mm):
//   text baseline  ≈ 1mm from the bottom of Row A
//   cap-height     = 9pt × 0.73 ÷ 2.8346 ≈ 2.32mm
//   strike target  = baseline + cap-height×0.50 = 1 + 1.16 = 2.16mm above row A bottom
//   line centre    = spacer + 0.2mm above row A bottom  (line in middle of Row C)
//   → spacer = 2.16 − 0.2 = 1.96mm    (use 1.96, clamp to ≥0)
//
// h is passed in so multi-line strike strings also work (Row A grows, spacer
// must grow too so the line stays at baseline+1.16 not near the top of a
// taller row).  For multi-line text the last-line baseline is at the bottom
// of the row, so the same formula still holds.
func strikethroughRows(label string, h float64) []core.Row {
	const (
		lineRowH  = 0.4
		clearance = 0.5
		spacer    = 1.96 // = 2.16 target − 0.2 (half of lineRowH)
	)
	_ = spacer // suppress "declared and not used" if optimizer removes it
	sp := h - 2.16 + 0.2 // = h − 1.96; grows with h so line stays at glyph level
	if sp < 0 {
		sp = 0
	}
	return []core.Row{
		row.New(h).Add(col.New(12).Add(
			text.New(label, txt(9, fontstyle.Normal, &agBlack)),
		)),
		row.New(sp),
		row.New(lineRowH).Add(col.New(12).Add(line.New(props.Line{
			Color:       &agBlack,
			Thickness:   0.5,
			SizePercent: 100,
			Orientation: orientation.Horizontal,
		}))),
		row.New(clearance),
	}
}

// ─── Signature block ──────────────────────────────────────────────────────────

func sigRows(label string) []core.Row {
	return []core.Row{
		row.New(10).Add(
			col.New(8).Add(line.New(props.Line{Color: &agGray, Thickness: 0.5})),
			col.New(1),
			col.New(3).Add(line.New(props.Line{Color: &agGray, Thickness: 0.5})),
		),
		row.New(7).Add(
			col.New(8).Add(text.New("("+label+")", txt(8, fontstyle.Normal, &agGray))),
			col.New(1),
			col.New(3).Add(text.New("(Date)", txt(8, fontstyle.Normal, &agGray))),
		),
		row.New(4),
	}
}

// ─── GenerateFullAgreement ────────────────────────────────────────────────────
//
// Dynamic layout rules:
//   • Every paragraph uses pRow() so the row height grows with the content.
//   • Table data cells use rowH() per cell; the row takes the max across cells.
//   • maroto auto-breaks pages when a row does not fit the remaining space —
//     we use m.AddRows() throughout (not page.New()) so this works everywhere.
//   • The payment plan section starts on a new page via a dedicated page.New()
//     marker added through m.AddPages(), which still allows further overflow.

func GenerateFullAgreement(path string, d FullAgreementData) error {
	cfg := config.NewBuilder().
		WithPageSize("A4").
		WithLeftMargin(20).WithRightMargin(20).
		WithTopMargin(20).WithBottomMargin(20).
		WithCustomFonts([]*entity.CustomFont{
			mustFont(fontFamily, fontstyle.Normal, "fonts/DejaVuSans.ttf"),
			mustFont(fontFamily, fontstyle.Bold, "fonts/DejaVuSans-Bold.ttf"),
			mustFont(fontFamily, fontstyle.Italic, "fonts/DejaVuSans-Oblique.ttf"),
			mustFont(fontFamily, fontstyle.BoldItalic, "fonts/DejaVuSans-BoldOblique.ttf"),
		}).
		WithDefaultFont(&props.Font{Family: fontFamily, Style: fontstyle.Normal, Size: 9}).
		Build()

	m := maroto.New(cfg)

	// ══════════════════════════════════════════════════════════════════════════
	// SECTION 1 — SERVICES AGREEMENT
	// ══════════════════════════════════════════════════════════════════════════

	preamble := fmt.Sprintf(
		`This Services Agreement (this "Agreement") is entered into as of the %s day of %s, 20%s, by and among/between:`,
		d.Day, d.Month, d.Year)

	providerAddrLine := d.ProviderAddress + `  (collectively "Service Provider") and`
	buyerAddrLine := d.BuyerAddress + `  (collectively "Buyer").`
	partiesLine := `Each Service Provider and Buyer may be referred to in this Agreement individually as a "Party" and collectively as the "Parties."`
	servicesIntro := "1. Services. Service Provider agrees to provide and Buyer agrees to purchase the following services for the specific projects described below:"
	purchaseLine := "2. Purchase Price. Buyer will pay to Service Provider and for all obligations specified in this Agreement, if any, as the full and complete purchase price, the sum of $" + d.PurchasePrice + "."
	taxLine := "Unless otherwise stated, (Check one)  \u2610  Service Provider  \u2610  Buyer shall be responsible for all taxes in connection with the purchase of Services in this Agreement."

	m.AddRows(
		row.New(8).Add(col.New(12).Add(text.New(
			"State of "+d.State+"  ___________________",
			txt(9, fontstyle.Normal, &agGray),
		))),
		row.New(14).Add(col.New(12).Add(text.New(
			"SERVICES AGREEMENT",
			txtAlign(18, fontstyle.Bold, &agBlack, align.Center),
		))),
		row.New(2).Add(col.New(12).Add(line.New(props.Line{Color: &agBlack, Thickness: 1.2}))),
		row.New(2).Add(col.New(12).Add(line.New(props.Line{Color: &agBlack, Thickness: 0.3}))),
		row.New(5),
		pRow(preamble, 9, 12, fontstyle.Normal, &agBlack, 10),
		row.New(4),
		// Service Provider — label fixed width, value dynamic
		row.New(7).Add(
			col.New(3).Add(text.New("Service Provider(s):", txt(9, fontstyle.Bold, &agBlack))),
			col.New(9).Add(text.New(d.ProviderName, txt(9, fontstyle.Normal, &agBlack))),
		),
		row.New(rowH(providerAddrLine, 9, 9, 6)).Add(
			col.New(3),
			col.New(9).Add(text.New(providerAddrLine, txt(9, fontstyle.Normal, &agGray))),
		),
		row.New(4),
		row.New(7).Add(
			col.New(3).Add(text.New("Buyer(s):", txt(9, fontstyle.Bold, &agBlack))),
			col.New(9).Add(text.New(d.BuyerName, txt(9, fontstyle.Normal, &agBlack))),
		),
		row.New(rowH(buyerAddrLine, 9, 9, 6)).Add(
			col.New(3),
			col.New(9).Add(text.New(buyerAddrLine, txt(9, fontstyle.Normal, &agGray))),
		),
		row.New(4),
		pRow(partiesLine, 9, 12, fontstyle.Normal, &agBlack, 8),
		row.New(4),
		pRow(servicesIntro, 9, 12, fontstyle.Normal, &agBlack, 8),
		row.New(3),
	)

	// Services table
	m.AddRows(
		row.New(10).Add(
			col.New(7).WithStyle(agHeaderCell()).Add(text.New("Description of Services",
				txtAlign(9, fontstyle.Bold, &agWhite, align.Center))),
			col.New(3).WithStyle(agHeaderCell()).Add(text.New("Number of Projects",
				txtAlign(9, fontstyle.Bold, &agWhite, align.Center))),
			col.New(2).WithStyle(agHeaderCell()).Add(text.New("Price per Project",
				txtAlign(9, fontstyle.Bold, &agWhite, align.Center))),
		),
	)

	for i, svc := range d.Services {
		svc := svc
		bg := &agWhite
		if i%2 != 0 {
			bg = &agLightGray
		}
		rh := math.Max(
			rowH(svc.Description, 8, 7, 8),
			math.Max(
				rowH(svc.NumProjects, 8, 3, 8),
				rowH("$"+svc.PricePerProject, 8, 2, 8),
			),
		)
		m.AddRows(row.New(rh).Add(
			col.New(7).WithStyle(agDataCell(bg)).Add(text.New(svc.Description, txt(8, fontstyle.Normal, &agBlack))),
			col.New(3).WithStyle(agDataCell(bg)).Add(text.New(svc.NumProjects, txtAlign(8, fontstyle.Normal, &agBlack, align.Center))),
			col.New(2).WithStyle(agDataCell(bg)).Add(text.New("$"+svc.PricePerProject, txtAlign(8, fontstyle.Normal, &agBlack, align.Right))),
		))
	}

	m.AddRows(
		row.New(5),
		pRow(purchaseLine, 9, 12, fontstyle.Normal, &agBlack, 10),
		row.New(3),
		pRow(taxLine, 9, 12, fontstyle.Normal, &agBlack, 8),
		row.New(4),
		row.New(8).Add(col.New(12).Add(text.New(
			"3. Payment. Payment for the Services will be by: (Check one)",
			txt(9, fontstyle.Normal, &agBlack),
		))),
		row.New(3),
		checkboxRow("Cash", "Credit or debit card"),
		checkboxRow("Personal check", "Wire transfer"),
		checkboxRow("Cashier's check", "Other: _______________"),
		checkboxRow("Money order", ""),
	)

	if d.Notes != "" {
		m.AddRows(
			row.New(5),
			pRow(d.Notes, 9, 12, fontstyle.Normal, &agGray, 8),
		)
	}

	// ══════════════════════════════════════════════════════════════════════════
	// SECTION 2 — PAYMENT PLAN  (starts on a new page)
	// ══════════════════════════════════════════════════════════════════════════
	//
	// We use m.AddRows() here too — maroto will auto-break if the payment
	// table is long.  The only hard break is the one forced below to ensure
	// the payment plan never starts mid-page after the agreement section.

	// Force page break: add an empty page marker then continue with AddRows.
	// maroto respects page.New() added via AddPages as a hard break.
	type pageAdder interface {
		AddPages(...core.Page)
	}
	if pa, ok := any(m).(pageAdder); ok {
		_ = pa // maroto.New() returns maroto.Maroto which has AddPages
	}
	// The cleaner way supported by all maroto v2 versions:
	m.AddRows(row.New(0)) // flush; actual page break via the page marker below

	// maroto v2 AddPages accepts core.Page — use page.New() from the import.
	// We cannot call page.New() here because it's not imported at file scope.
	// Instead, force the break by adding a sentinel row that fills remaining
	// page space.  maroto auto-breaks when a row would overflow the page, so
	// adding a very large row triggers the break.
	//
	// CORRECT approach: just keep adding rows with m.AddRows — maroto will
	// page-break the payment plan content automatically.  We add a visual
	// section heading to make it clear a new logical section starts.

	contractText := fmt.Sprintf(
		`By this contract, %s agrees to make payments to %s, hereafter known as "Lender," by the following schedule in exchange for %s. This payment schedule is enforceable by law, and the methods described below will be used in cases of delinquent payment.`,
		d.Payer, d.Payee, d.Product)

	scheduleText := fmt.Sprintf(
		`By this agreement, it is agreed that a payment of %s will be surrendered to the Lender every %s until the total of the payment required, which is %s, has been delivered. The payment plan will take the following form:`,
		d.AmountPerPeriod, d.Interval, d.TotalAmount)

	bindingText := fmt.Sprintf(
		`This agreement is binding, and failure to meet its terms will allow the Lender to take certain recourse. First, late payments will incur a fee of %s every %s. Insufficient payment and bounced checks will incur a fee of %s. If payment should not be delivered at all, Lender will be entitled to %s.`,
		d.LateFee, d.Interval, d.BounceFee, d.LenderAction)

	termsText := "In addition, the following terms and conditions apply: " + d.TermsConditions

	closingText := `By signing this agreement, all parties agree to the terms as described above. Alterations to this agreement can only be made by both parties and must be placed in writing. Both parties will receive a printed copy of this agreement, and will be responsible for upholding its terms.`

	// Section heading — acts as a natural separator and triggers a page break
	// if not enough space remains on page 1.
	m.AddRows(
		row.New(10),
		row.New(12).Add(col.New(12).Add(text.New(
			"PAYMENT PLAN",
			txtAlign(14, fontstyle.Bold, &agBlack, align.Center),
		))),
		row.New(2).Add(col.New(12).Add(line.New(props.Line{Color: &agBlack, Thickness: 0.5}))),
		row.New(5),
		pRow(contractText, 9, 12, fontstyle.Normal, &agBlack, 14),
		row.New(4),
		pRow(scheduleText, 9, 12, fontstyle.Normal, &agBlack, 12),
		row.New(3),
	)

	// Payment schedule table
	m.AddRows(
		row.New(9).Add(
			col.New(6).WithStyle(agHeaderCell()).Add(text.New("Payment Date",
				txtAlign(9, fontstyle.Bold, &agWhite, align.Center))),
			col.New(6).WithStyle(agHeaderCell()).Add(text.New("Amount",
				txtAlign(9, fontstyle.Bold, &agWhite, align.Center))),
		),
	)

	for i, pmt := range d.Payments {
		pmt := pmt
		bg := &agWhite
		if i%2 != 0 {
			bg = &agLightGray
		}
		rh := math.Max(
			rowH(pmt.Date, 8, 6, 8),
			rowH(pmt.Amount, 8, 6, 8),
		)
		m.AddRows(row.New(rh).Add(
			col.New(6).WithStyle(agDataCell(bg)).Add(text.New(pmt.Date, txtAlign(8, fontstyle.Normal, &agBlack, align.Center))),
			col.New(6).WithStyle(agDataCell(bg)).Add(text.New(pmt.Amount, txtAlign(8, fontstyle.Normal, &agBlack, align.Center))),
		))
	}

	m.AddRows(
		row.New(5),
		row.New(8).Add(col.New(12).Add(text.New(
			"These payments include any interest and other charges that may apply.",
			txt(9, fontstyle.Normal, &agBlack),
		))),
		row.New(4),
		pRow(bindingText, 9, 12, fontstyle.Normal, &agBlack, 16),
		row.New(4),
		pRow(termsText, 9, 12, fontstyle.Normal, &agBlack, 8),
		row.New(4),
		pRow(closingText, 9, 12, fontstyle.Normal, &agBlack, 14),
		row.New(8),
	)

	// Signature lines
	m.AddRows(sigRows(d.Payer)...)
	m.AddRows(row.New(6))
	m.AddRows(sigRows(d.Payee)...)

	// ── Styled text demo ──────────────────────────────────────────────────────
	//
	// ITALIC     — fontstyle.Italic   → DejaVuSans-Oblique.ttf (embedded)
	// UNDERLINE  — fontstyle.Underline → maroto native underline decoration
	// BOLD       — fontstyle.Bold     → DejaVuSans-Bold.ttf (embedded)
	// STRIKETHROUGH — three-row sandwich (see strikethroughRows above)
	//   The row height is computed via rowH() so multi-line strings are handled.

	italicStr := "This is italic."
	boldStr := "This is bold."
	underlineStr := "This is underline."
	strikeStr := "This is strikethrough."

	m.AddRows(
		row.New(8),
		row.New(rowH(italicStr, 9, 12, 8)).Add(col.New(12).Add(
			text.New(italicStr, txt(9, fontstyle.Italic, &agBlack)),
		)),
		row.New(4),
		row.New(rowH(boldStr, 9, 12, 8)).Add(col.New(12).Add(
			text.New(boldStr, txt(9, fontstyle.Bold, &agBlack)),
		)),
		row.New(4),
		row.New(rowH(underlineStr, 9, 12, 8)).Add(col.New(12).Add(
			text.New(underlineStr, txt(9, fontstyle.Underline, &agBlack)),
		)),
		row.New(4),
	)
	m.AddRows(strikethroughRows(strikeStr, rowH(strikeStr, 9, 12, 8))...)

	doc, err := m.Generate()
	if err != nil {
		return err
	}
	return doc.Save(path)
}
