package mapdf

import (
	"embed"
	"fmt"

	"github.com/johnfercher/maroto/v2"
	"github.com/johnfercher/maroto/v2/pkg/components/col"
	"github.com/johnfercher/maroto/v2/pkg/components/line"
	"github.com/johnfercher/maroto/v2/pkg/components/page"
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

// ─── Shared types ─────────────────────────────────────────────────────────────

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

func txt(size float64, style fontstyle.Type, clr *props.Color) props.Text {
	return props.Text{Size: size, Style: style, Color: clr, Family: fontFamily}
}

func txtAlign(size float64, style fontstyle.Type, clr *props.Color, a align.Type) props.Text {
	return props.Text{Size: size, Style: style, Color: clr, Align: a, Family: fontFamily}
}

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

// ─── strikethroughRows ────────────────────────────────────────────────────────
//
// maroto draws line.New() at the VERTICAL CENTRE of the row it occupies.
// There is no z-layering, so the line and text must be in separate rows.
//
// Strategy — three-row sandwich:
//
//   Row A  h=8mm   the text  (baseline sits near the bottom of this row)
//   Row B  h=Spt   spacer    moves the cursor UP relative to the text baseline
//   Row C  h=0.4pt line row  line.New() draws at the centre → mid-glyph
//
// The spacer height S is chosen so that Row C's centre coincides with the
// strikethrough position for DejaVu Sans 9pt:
//   cap-height ≈ 9 × 0.73 = 6.57pt
//   strikethrough at 50% cap-height = 3.3pt above baseline
//
// maroto row height is in mm internally but the API accepts float64 directly.
// In the PDF output 1 maroto unit ≈ 1pt.  With a text row of 8 and the
// baseline sitting ~1pt from the bottom, we need the line at baseline+3.3pt,
// i.e. 4.3pt from the bottom of Row A.  The line sits at centre of Row C
// (0.4/2 = 0.2pt).  So spacer = 8 - 4.3 - 0.2 = 3.5pt → use 3.5.
//
// A tiny bottom-clearance row of 0.5pt keeps adjacent content from touching
// the rule.

func strikethroughRows(label string) []core.Row {
	strikeLineProps := props.Line{
		Color:       &agBlack,
		Thickness:   0.5,
		SizePercent: 100,
		Orientation: orientation.Horizontal,
	}
	return []core.Row{
		// Row A: the text
		row.New(8).Add(col.New(12).Add(
			text.New(label, txt(9, fontstyle.Normal, &agBlack)),
		)),
		// Row B: spacer — pushes cursor so Row C's centre lands at mid cap-height
		row.New(3.5),
		// Row C: the strikethrough rule (line drawn at row centre = 0.2pt into row)
		row.New(0.4).Add(col.New(12).Add(
			line.New(strikeLineProps),
		)),
		// bottom clearance
		row.New(0.5),
	}
}

// ─── GenerateFullAgreement ────────────────────────────────────────────────────

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
	// PAGE 1 — SERVICES AGREEMENT
	// ══════════════════════════════════════════════════════════════════════════
	p1 := page.New()

	p1.Add(
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
		row.New(12).Add(col.New(12).Add(text.New(
			fmt.Sprintf(`This Services Agreement (this "Agreement") is entered into as of the %s day of %s, 20%s, by and among/between:`,
				d.Day, d.Month, d.Year),
			txt(9, fontstyle.Normal, &agBlack),
		))),
		row.New(4),
		row.New(7).Add(
			col.New(3).Add(text.New("Service Provider(s):", txt(9, fontstyle.Bold, &agBlack))),
			col.New(9).Add(text.New(d.ProviderName, txt(9, fontstyle.Normal, &agBlack))),
		),
		row.New(6).Add(
			col.New(3),
			col.New(9).Add(text.New(d.ProviderAddress+`  (collectively "Service Provider") and`, txt(9, fontstyle.Normal, &agGray))),
		),
		row.New(4),
		row.New(7).Add(
			col.New(3).Add(text.New("Buyer(s):", txt(9, fontstyle.Bold, &agBlack))),
			col.New(9).Add(text.New(d.BuyerName, txt(9, fontstyle.Normal, &agBlack))),
		),
		row.New(6).Add(
			col.New(3),
			col.New(9).Add(text.New(d.BuyerAddress+`  (collectively "Buyer").`, txt(9, fontstyle.Normal, &agGray))),
		),
		row.New(4),
		row.New(10).Add(col.New(12).Add(text.New(
			`Each Service Provider and Buyer may be referred to in this Agreement individually as a "Party" and collectively as the "Parties."`,
			txt(9, fontstyle.Normal, &agBlack),
		))),
		row.New(4),
		row.New(10).Add(col.New(12).Add(text.New(
			"1. Services. Service Provider agrees to provide and Buyer agrees to purchase the following services for the specific projects described below:",
			txt(9, fontstyle.Normal, &agBlack),
		))),
		row.New(3),
	)

	p1.Add(
		row.New(10).Add(
			col.New(7).WithStyle(agHeaderCell()).Add(text.New("Description of Services",
				txtAlign(9, fontstyle.Bold, &agWhite, align.Center))),
			col.New(3).WithStyle(agHeaderCell()).Add(text.New("Number of Projects",
				txtAlign(9, fontstyle.Bold, &agWhite, align.Center))),
			col.New(2).WithStyle(agHeaderCell()).Add(text.New("Price per Project",
				txtAlign(9, fontstyle.Bold, &agWhite, align.Center))),
		),
	)

	var svcRows []core.Row
	for i, svc := range d.Services {
		svc := svc
		bg := &agWhite
		if i%2 != 0 {
			bg = &agLightGray
		}
		svcRows = append(svcRows, row.New(8).Add(
			col.New(7).WithStyle(agDataCell(bg)).Add(text.New(svc.Description, txt(8, fontstyle.Normal, &agBlack))),
			col.New(3).WithStyle(agDataCell(bg)).Add(text.New(svc.NumProjects, txtAlign(8, fontstyle.Normal, &agBlack, align.Center))),
			col.New(2).WithStyle(agDataCell(bg)).Add(text.New("$"+svc.PricePerProject, txtAlign(8, fontstyle.Normal, &agBlack, align.Right))),
		))
	}
	p1.Add(svcRows...)

	p1.Add(
		row.New(5),
		row.New(12).Add(col.New(12).Add(text.New(
			"2. Purchase Price. Buyer will pay to Service Provider and for all obligations specified in this Agreement, if any, as the full and complete purchase price, the sum of $"+d.PurchasePrice+".",
			txt(9, fontstyle.Normal, &agBlack),
		))),
		row.New(3),
		row.New(10).Add(col.New(12).Add(text.New(
			"Unless otherwise stated, (Check one)  \u2610  Service Provider  \u2610  Buyer shall be responsible for all taxes in connection with the purchase of Services in this Agreement.",
			txt(9, fontstyle.Normal, &agBlack),
		))),
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
		p1.Add(
			row.New(5),
			row.New(10).Add(col.New(12).Add(text.New(d.Notes, txt(9, fontstyle.Normal, &agGray)))),
		)
	}

	m.AddPages(p1)

	// ══════════════════════════════════════════════════════════════════════════
	// PAGE 2 — PAYMENT PLAN
	// ══════════════════════════════════════════════════════════════════════════
	p2 := page.New()

	p2.Add(
		row.New(18).Add(col.New(12).Add(text.New(
			fmt.Sprintf(`By this contract, %s agrees to make payments to %s, hereafter known as "Lender," by the following schedule in exchange for %s. This payment schedule is enforceable by law, and the methods described below will be used in cases of delinquent payment.`,
				d.Payer, d.Payee, d.Product),
			txt(9, fontstyle.Normal, &agBlack),
		))),
		row.New(4),
		row.New(16).Add(col.New(12).Add(text.New(
			fmt.Sprintf(`By this agreement, it is agreed that a payment of %s will be surrendered to the Lender every %s until the total of the payment required, which is %s, has been delivered. The payment plan will take the following form:`,
				d.AmountPerPeriod, d.Interval, d.TotalAmount),
			txt(9, fontstyle.Normal, &agBlack),
		))),
		row.New(3),
	)

	p2.Add(
		row.New(9).Add(
			col.New(6).WithStyle(agHeaderCell()).Add(text.New("Payment Date",
				txtAlign(9, fontstyle.Bold, &agWhite, align.Center))),
			col.New(6).WithStyle(agHeaderCell()).Add(text.New("Amount",
				txtAlign(9, fontstyle.Bold, &agWhite, align.Center))),
		),
	)

	var pmtRows []core.Row
	for i, pmt := range d.Payments {
		pmt := pmt
		bg := &agWhite
		if i%2 != 0 {
			bg = &agLightGray
		}
		pmtRows = append(pmtRows, row.New(8).Add(
			col.New(6).WithStyle(agDataCell(bg)).Add(text.New(pmt.Date, txtAlign(8, fontstyle.Normal, &agBlack, align.Center))),
			col.New(6).WithStyle(agDataCell(bg)).Add(text.New(pmt.Amount, txtAlign(8, fontstyle.Normal, &agBlack, align.Center))),
		))
	}
	p2.Add(pmtRows...)

	p2.Add(
		row.New(5),
		row.New(10).Add(col.New(12).Add(text.New(
			"These payments include any interest and other charges that may apply.",
			txt(9, fontstyle.Normal, &agBlack),
		))),
		row.New(4),
		row.New(24).Add(col.New(12).Add(text.New(
			fmt.Sprintf(`This agreement is binding, and failure to meet its terms will allow the Lender to take certain recourse. First, late payments will incur a fee of %s every %s. Insufficient payment and bounced checks will incur a fee of %s. If payment should not be delivered at all, Lender will be entitled to %s.`,
				d.LateFee, d.Interval, d.BounceFee, d.LenderAction),
			txt(9, fontstyle.Normal, &agBlack),
		))),
		row.New(4),
		row.New(8).Add(col.New(12).Add(text.New(
			"In addition, the following terms and conditions apply: "+d.TermsConditions,
			txt(9, fontstyle.Normal, &agBlack),
		))),
		row.New(4),
		row.New(20).Add(col.New(12).Add(text.New(
			`By signing this agreement, all parties agree to the terms as described above. Alterations to this agreement can only be made by both parties and must be placed in writing. Both parties will receive a printed copy of this agreement, and will be responsible for upholding its terms.`,
			txt(9, fontstyle.Normal, &agBlack),
		))),
		row.New(8),
	)

	// ── Styled text demo ──────────────────────────────────────────────────────
	//
	// Italic: fontstyle.Italic selects DejaVuSans-Oblique.ttf — already embedded.
	//
	// Bold: fontstyle.Bold selects DejaVuSans-Bold.ttf — already embedded.
	//
	// Strikethrough: uses strikethroughRows() which places a line.New() rule
	// in a separate 0.4pt row below a calibrated spacer so it lands at the
	// correct vertical position relative to the text glyphs above it.
	// line.New() always draws at the vertical centre of its row, so:
	//   text row (8pt) → spacer (3.5pt) → line row (0.4pt, centre = 0.2pt)
	//   line lands at: 0.2pt into row C = (8 - 3.5 - 0.2) = 4.3pt above
	//   the bottom of the text row ≈ 50% of DejaVu Sans 9pt cap-height. ✓
	p2.Add(
		row.New(6),
		// italic
		row.New(8).Add(col.New(12).Add(
			text.New("This is italic.", txt(9, fontstyle.Italic, &agBlack)),
		)),
		row.New(4),
		// bold
		row.New(8).Add(col.New(12).Add(
			text.New("This is bold.", txt(9, fontstyle.Bold, &agBlack)),
		)),
		row.New(4),
	)
	// strikethrough — uses the sandwich helper
	p2.Add(strikethroughRows("This is strikethrough.")...)

	// ── Signature lines ───────────────────────────────────────────────────────
	sigLine := func(label string) []core.Row {
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
	p2.Add(row.New(8))
	p2.Add(sigLine(d.Payer)...)
	p2.Add(row.New(6))
	p2.Add(sigLine(d.Payee)...)

	m.AddPages(p2)

	doc, err := m.Generate()
	if err != nil {
		return err
	}
	return doc.Save(path)
}
