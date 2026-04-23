package pdf

import (
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
	"github.com/johnfercher/maroto/v2/pkg/core"
	"github.com/johnfercher/maroto/v2/pkg/props"
)

// ─── Shared types ──────────────────────────────────────────────────────────────

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
	// Page 1 — Services Agreement
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

	// Page 2 — Payment Plan
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

// ─── Shared colors ─────────────────────────────────────────────────────────────

var (
	agBlack     = props.Color{Red: 20, Green: 20, Blue: 20}
	agGray      = props.Color{Red: 110, Green: 110, Blue: 110}
	agLightGray = props.Color{Red: 230, Green: 230, Blue: 230}
	agWhite     = props.Color{Red: 255, Green: 255, Blue: 255}
	agBorder    = props.Color{Red: 160, Green: 160, Blue: 160}
	agHeaderBg  = props.Color{Red: 50, Green: 50, Blue: 50}
)

// ─── Cell style helpers ────────────────────────────────────────────────────────

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

// ─── GenerateFullAgreement ────────────────────────────────────────────────────

func GenerateFullAgreement(path string, d FullAgreementData) error {
	cfg := config.NewBuilder().
		WithPageSize("A4").
		WithLeftMargin(20).WithRightMargin(20).
		WithTopMargin(20).WithBottomMargin(20).
		Build()

	m := maroto.New(cfg)

	// ══════════════════════════════════════════════════════════════════════════
	// PAGE 1 — SERVICES AGREEMENT
	// ══════════════════════════════════════════════════════════════════════════
	p1 := page.New()

	p1.Add(
		// State
		row.New(8).Add(col.New(12).Add(text.New(
			"State of "+d.State+"  ___________________",
			props.Text{Size: 9, Color: &agGray},
		))),

		// Title
		row.New(14).Add(col.New(12).Add(text.New(
			"SERVICES AGREEMENT",
			props.Text{Size: 18, Style: fontstyle.Bold, Align: align.Center, Color: &agBlack},
		))),

		// Double divider
		row.New(2).Add(col.New(12).Add(line.New(props.Line{Color: &agBlack, Thickness: 1.2}))),
		row.New(2).Add(col.New(12).Add(line.New(props.Line{Color: &agBlack, Thickness: 0.3}))),
		row.New(5),

		// Intro
		row.New(12).Add(col.New(12).Add(text.New(
			fmt.Sprintf(`This Services Agreement (this "Agreement") is entered into as of the %s day of %s, 20%s, by and among/between:`,
				d.Day, d.Month, d.Year),
			props.Text{Size: 9, Color: &agBlack},
		))),
		row.New(4),

		// Service Provider
		row.New(7).Add(col.New(2).Add(text.New("Service Provider(s):", props.Text{Size: 9, Style: fontstyle.Bold, Color: &agBlack})),
			col.New(10).Add(text.New(d.ProviderName, props.Text{Size: 9, Color: &agBlack}))),
		row.New(6).Add(col.New(2), col.New(10).Add(text.New(
			d.ProviderAddress+`  (collectively "Service Provider") and`,
			props.Text{Size: 9, Color: &agGray},
		))),
		row.New(4),

		// Buyer
		row.New(7).Add(col.New(2).Add(text.New("Buyer(s):", props.Text{Size: 9, Style: fontstyle.Bold, Color: &agBlack})),
			col.New(10).Add(text.New(d.BuyerName, props.Text{Size: 9, Color: &agBlack}))),
		row.New(6).Add(col.New(2), col.New(10).Add(text.New(
			d.BuyerAddress+`  (collectively "Buyer").`,
			props.Text{Size: 9, Color: &agGray},
		))),
		row.New(4),

		// Party clause
		row.New(10).Add(col.New(12).Add(text.New(
			`Each Service Provider and Buyer may be referred to in this Agreement individually as a "Party" and collectively as the "Parties."`,
			props.Text{Size: 9, Color: &agBlack},
		))),
		row.New(4),

		// Section 1 header
		row.New(10).Add(col.New(12).Add(text.New(
			"1. Services. Service Provider agrees to provide and Buyer agrees to purchase the following services for the specific projects described below:",
			props.Text{Size: 9, Color: &agBlack},
		))),
		row.New(3),
	)

	// ── Services table ────────────────────────────────────────────────────────
	p1.Add(
		row.New(10).Add(
			col.New(7).WithStyle(agHeaderCell()).Add(text.New("Description of Services",
				props.Text{Size: 9, Style: fontstyle.Bold, Align: align.Center, Color: &agWhite})),
			col.New(3).WithStyle(agHeaderCell()).Add(text.New("Number of Projects",
				props.Text{Size: 9, Style: fontstyle.Bold, Align: align.Center, Color: &agWhite})),
			col.New(2).WithStyle(agHeaderCell()).Add(text.New("Price per Project",
				props.Text{Size: 9, Style: fontstyle.Bold, Align: align.Center, Color: &agWhite})),
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
			col.New(7).WithStyle(agDataCell(bg)).Add(text.New(svc.Description,
				props.Text{Size: 8, Color: &agBlack})),
			col.New(3).WithStyle(agDataCell(bg)).Add(text.New(svc.NumProjects,
				props.Text{Size: 8, Align: align.Center, Color: &agBlack})),
			col.New(2).WithStyle(agDataCell(bg)).Add(text.New("$"+svc.PricePerProject,
				props.Text{Size: 8, Align: align.Right, Color: &agBlack})),
		))
	}
	p1.Add(svcRows...)

	p1.Add(
		row.New(5),

		// Section 2
		row.New(12).Add(col.New(12).Add(text.New(
			"2. Purchase Price. Buyer will pay to Service Provider and for all obligations specified in this Agreement, if any, as the full and complete purchase price, the sum of $"+d.PurchasePrice+".",
			props.Text{Size: 9, Color: &agBlack},
		))),
		row.New(3),
		row.New(12).Add(col.New(12).Add(text.New(
			"Unless otherwise stated, (Check one)  [ ]  Service Provider  [ ]  Buyer shall be responsible for all taxes in connection with the purchase of Services in this Agreement.",
			props.Text{Size: 9, Color: &agBlack},
		))),
		row.New(4),

		// Section 3
		row.New(8).Add(col.New(12).Add(text.New(
			"3. Payment. Payment for the Services will be by: (Check one)",
			props.Text{Size: 9, Color: &agBlack},
		))),
		row.New(3),
	)

	// ── Payment method checkbox table ─────────────────────────────────────────
	// 2-column layout with borders
	type checkboxPair struct{ left, right string }
	checkboxes := []checkboxPair{
		{"[ ]  Cash", "[ ]  Credit or debit card"},
		{"[ ]  Personal check", "[ ]  Wire transfer"},
		{"[ ]  Cashier's check", "[ ]  Other: _______________"},
		{"[ ]  Money order", ""},
	}
	for _, cb := range checkboxes {
		cb := cb
		p1.Add(row.New(8).Add(
			col.New(6).WithStyle(agDataCell(&agWhite)).Add(text.New(cb.left, props.Text{Size: 9, Color: &agBlack})),
			col.New(6).WithStyle(agDataCell(&agWhite)).Add(text.New(cb.right, props.Text{Size: 9, Color: &agBlack})),
		))
	}

	if d.Notes != "" {
		p1.Add(
			row.New(5),
			row.New(10).Add(col.New(12).Add(text.New(d.Notes, props.Text{Size: 9, Color: &agGray}))),
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
			props.Text{Size: 9, Color: &agBlack},
		))),
		row.New(4),
		row.New(16).Add(col.New(12).Add(text.New(
			fmt.Sprintf(`By this agreement, it is agreed that a payment of %s will be surrendered to the Lender every %s until the total of the payment required, which is %s, has been delivered. The payment plan will take the following form:`,
				d.AmountPerPeriod, d.Interval, d.TotalAmount),
			props.Text{Size: 9, Color: &agBlack},
		))),
		row.New(3),
	)

	// ── Payment schedule table ────────────────────────────────────────────────
	p2.Add(
		row.New(9).Add(
			col.New(6).WithStyle(agHeaderCell()).Add(text.New("Payment Date",
				props.Text{Size: 9, Style: fontstyle.Bold, Align: align.Center, Color: &agWhite})),
			col.New(6).WithStyle(agHeaderCell()).Add(text.New("Amount",
				props.Text{Size: 9, Style: fontstyle.Bold, Align: align.Center, Color: &agWhite})),
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
			col.New(6).WithStyle(agDataCell(bg)).Add(text.New(pmt.Date,
				props.Text{Size: 8, Align: align.Center, Color: &agBlack})),
			col.New(6).WithStyle(agDataCell(bg)).Add(text.New(pmt.Amount,
				props.Text{Size: 8, Align: align.Center, Color: &agBlack})),
		))
	}
	p2.Add(pmtRows...)

	p2.Add(
		row.New(5),
		row.New(10).Add(col.New(12).Add(text.New(
			"These payments include any interest and other charges that may apply.",
			props.Text{Size: 9, Color: &agBlack},
		))),
		row.New(4),
		row.New(24).Add(col.New(12).Add(text.New(
			fmt.Sprintf(`This agreement is binding, and failure to meet its terms will allow the Lender to take certain recourse. First, late payments will incur a fee of %s every %s. Insufficient payment and bounced checks will incur a fee of %s. If payment should not be delivered at all, Lender will be entitled to %s.`,
				d.LateFee, d.Interval, d.BounceFee, d.LenderAction),
			props.Text{Size: 9, Color: &agBlack},
		))),
		row.New(4),
		row.New(8).Add(col.New(12).Add(text.New(
			"In addition, the following terms and conditions apply: "+d.TermsConditions,
			props.Text{Size: 9, Color: &agBlack},
		))),
		row.New(4),
		row.New(20).Add(col.New(12).Add(text.New(
			`By signing this agreement, all parties agree to the terms as described above. Alterations to this agreement can only be made by both parties and must be placed in writing. Both parties will receive a printed copy of this agreement, and will be responsible for upholding its terms.`,
			props.Text{Size: 9, Color: &agBlack},
		))),
		row.New(10),
	)

	// ── Signature table ───────────────────────────────────────────────────────
	p2.Add(
		row.New(14).Add(
			col.New(5).WithStyle(agDataCell(&agWhite)).Add(text.New(
				"\n\n("+d.Payer+")", props.Text{Size: 8, Color: &agGray})),
			col.New(2).WithStyle(agDataCell(&agWhite)).Add(text.New(
				"\n\n(Date)", props.Text{Size: 8, Color: &agGray})),
			col.New(1),
			col.New(3).WithStyle(agDataCell(&agWhite)).Add(text.New(
				"\n\n("+d.Payee+")", props.Text{Size: 8, Color: &agGray})),
			col.New(1).WithStyle(agDataCell(&agWhite)).Add(text.New(
				"\n\n(Date)", props.Text{Size: 8, Color: &agGray})),
		),
	)

	m.AddPages(p2)

	doc, err := m.Generate()
	if err != nil {
		return err
	}
	return doc.Save(path)
}
