package mapdf

import (
	"fmt"
	"os"

	"github.com/gpdf-dev/gpdf"
	"github.com/gpdf-dev/gpdf/document"
	"github.com/gpdf-dev/gpdf/pdf"
	"github.com/gpdf-dev/gpdf/template"
)

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

// ─── Colors ───────────────────────────────────────────────────────────────────

var (
	cBlack     = pdf.RGB(0.08, 0.08, 0.08)
	cGray      = pdf.RGB(0.43, 0.43, 0.43)
	cLightGray = pdf.RGB(0.90, 0.90, 0.90)
	cHeaderBg  = pdf.RGB(0.20, 0.20, 0.20)
	cBorder    = pdf.RGB(0.63, 0.63, 0.63)
	cWhite     = pdf.RGB(1, 1, 1)
)

// ─── Helper: full-width divider line ─────────────────────────────────────────

func dividerRow(page *template.PageBuilder, thickness float64, clr pdf.Color) {
	page.AutoRow(func(r *template.RowBuilder) {
		r.Col(12, func(c *template.ColBuilder) {
			c.Line(
				template.LineThickness(document.Pt(thickness)),
				template.LineColor(clr),
			)
		})
	})
}

// ─── Helper: spacer row ───────────────────────────────────────────────────────

func spacer(page *template.PageBuilder, mm float64) {
	page.Row(document.Mm(mm), func(r *template.RowBuilder) {
		r.Col(12, func(c *template.ColBuilder) {})
	})
}

// ─── Helper: body paragraph ───────────────────────────────────────────────────

func para(page *template.PageBuilder, content string, opts ...template.ColOption) {
	page.AutoRow(func(r *template.RowBuilder) {
		r.Col(12, func(c *template.ColBuilder) {
			c.Text(content, opts...)
		})
	})
}

// ─── Helper: two-column label + value row ────────────────────────────────────

func labelValue(page *template.PageBuilder, label, value string) {
	page.AutoRow(func(r *template.RowBuilder) {
		r.Col(3, func(c *template.ColBuilder) {
			c.Text(label, template.Bold(), template.FontSize(9))
		})
		r.Col(9, func(c *template.ColBuilder) {
			c.Text(value, template.FontSize(9))
		})
	})
}

// ─── Helper: checkbox row ─────────────────────────────────────────────────────

func checkboxRow(page *template.PageBuilder, left, right string) {
	page.Row(document.Mm(7), func(r *template.RowBuilder) {
		r.Col(1, func(c *template.ColBuilder) {})
		r.Col(5, func(c *template.ColBuilder) {
			c.Text("\u2610  "+left, template.FontSize(9))
		})
		r.Col(5, func(c *template.ColBuilder) {
			if right != "" {
				c.Text("\u2610  "+right, template.FontSize(9))
			}
		})
		r.Col(1, func(c *template.ColBuilder) {})
	})
}

// ─── Helper: table header ─────────────────────────────────────────────────────

type colDef struct {
	label string
	span  int
}

func tableHeader(page *template.PageBuilder, cols []colDef) {
	page.Row(document.Mm(9), func(r *template.RowBuilder) {
		for _, cd := range cols {
			cd := cd
			r.Col(cd.span, func(c *template.ColBuilder) {
				c.Text(cd.label,
					template.Bold(),
					template.FontSize(9),
					template.TextColor(cWhite),
					template.Align(document.AlignCenter),
					template.BackgroundColor(cHeaderBg),
				)
			})
		}
	})
}

// ─── Helper: table data row ───────────────────────────────────────────────────

type cellDef struct {
	text  string
	span  int
	align document.Alignment
}

func tableDataRow(page *template.PageBuilder, cells []cellDef, bg pdf.Color) {
	page.AutoRow(func(r *template.RowBuilder) {
		for _, cd := range cells {
			cd := cd
			r.Col(cd.span, func(c *template.ColBuilder) {
				c.Text(cd.text,
					template.FontSize(8),
					template.Align(cd.align),
					template.BackgroundColor(bg),
				)
			})
		}
	})
}

// ─── Helper: signature block ─────────────────────────────────────────────────

func sigBlock(page *template.PageBuilder, label string) {
	page.Row(document.Mm(10), func(r *template.RowBuilder) {
		r.Col(8, func(c *template.ColBuilder) {
			c.Line(template.LineColor(cGray), template.LineThickness(document.Pt(0.5)))
		})
		r.Col(1, func(c *template.ColBuilder) {})
		r.Col(3, func(c *template.ColBuilder) {
			c.Line(template.LineColor(cGray), template.LineThickness(document.Pt(0.5)))
		})
	})
	page.Row(document.Mm(6), func(r *template.RowBuilder) {
		r.Col(8, func(c *template.ColBuilder) {
			c.Text("("+label+")", template.FontSize(8), template.TextColor(cGray))
		})
		r.Col(1, func(c *template.ColBuilder) {})
		r.Col(3, func(c *template.ColBuilder) {
			c.Text("(Date)", template.FontSize(8), template.TextColor(cGray))
		})
	})
	spacer(page, 4)
}

// ─── GenerateFullAgreement ────────────────────────────────────────────────────

// GenerateFullAgreement creates a two-section PDF using github.com/gpdf-dev/gpdf:
//   Section 1 — Services Agreement
//   Section 2 — Payment Plan (new page)
//
// go get github.com/gpdf-dev/gpdf
func GenerateFullAgreement(path string, d FullAgreementData) error {
	doc := gpdf.NewDocument(
		gpdf.WithPageSize(gpdf.A4),
		gpdf.WithMargins(document.UniformEdges(document.Mm(20))),
	)

	// ══════════════════════════════════════════════════════════════════════════
	// PAGE 1 — SERVICES AGREEMENT
	// ══════════════════════════════════════════════════════════════════════════
	p1 := doc.AddPage()

	// State line
	p1.AutoRow(func(r *template.RowBuilder) {
		r.Col(12, func(c *template.ColBuilder) {
			c.Text("State of "+d.State+"  ___________________",
				template.FontSize(9), template.TextColor(cGray))
		})
	})

	// Title
	p1.AutoRow(func(r *template.RowBuilder) {
		r.Col(12, func(c *template.ColBuilder) {
			c.Text("SERVICES AGREEMENT",
				template.FontSize(18),
				template.Bold(),
				template.Align(document.AlignCenter),
			)
		})
	})

	// Double divider
	dividerRow(p1, 2.0, cBlack)
	dividerRow(p1, 0.5, cBlack)
	spacer(p1, 4)

	// Preamble
	para(p1, fmt.Sprintf(
		`This Services Agreement (this "Agreement") is entered into as of the %s day of %s, 20%s, by and among/between:`,
		d.Day, d.Month, d.Year),
		template.FontSize(9),
	)
	spacer(p1, 3)

	// Service Provider
	labelValue(p1, "Service Provider(s):", d.ProviderName)
	p1.AutoRow(func(r *template.RowBuilder) {
		r.Col(3, func(c *template.ColBuilder) {})
		r.Col(9, func(c *template.ColBuilder) {
			c.Text(d.ProviderAddress+`  (collectively "Service Provider") and`,
				template.FontSize(9), template.TextColor(cGray))
		})
	})
	spacer(p1, 3)

	// Buyer
	labelValue(p1, "Buyer(s):", d.BuyerName)
	p1.AutoRow(func(r *template.RowBuilder) {
		r.Col(3, func(c *template.ColBuilder) {})
		r.Col(9, func(c *template.ColBuilder) {
			c.Text(d.BuyerAddress+`  (collectively "Buyer").`,
				template.FontSize(9), template.TextColor(cGray))
		})
	})
	spacer(p1, 3)

	// Parties clause
	para(p1,
		`Each Service Provider and Buyer may be referred to in this Agreement individually as a "Party" and collectively as the "Parties."`,
		template.FontSize(9),
	)
	spacer(p1, 3)

	// Section 1 intro
	para(p1,
		"1. Services. Service Provider agrees to provide and Buyer agrees to purchase the following services for the specific projects described below:",
		template.FontSize(9),
	)
	spacer(p1, 2)

	// Services table
	tableHeader(p1, []colDef{
		{"Description of Services", 7},
		{"Number of Projects", 3},
		{"Price per Project", 2},
	})

	for i, svc := range d.Services {
		bg := cWhite
		if i%2 != 0 {
			bg = cLightGray
		}
		tableDataRow(p1, []cellDef{
			{svc.Description, 7, document.AlignLeft},
			{svc.NumProjects, 3, document.AlignCenter},
			{"$" + svc.PricePerProject, 2, document.AlignRight},
		}, bg)
	}

	spacer(p1, 4)

	// Section 2: Purchase Price
	para(p1,
		"2. Purchase Price. Buyer will pay to Service Provider and for all obligations specified in this Agreement, if any, as the full and complete purchase price, the sum of $"+d.PurchasePrice+".",
		template.FontSize(9),
	)
	spacer(p1, 3)

	// Tax checkbox line
	para(p1,
		"Unless otherwise stated, (Check one)  \u2610  Service Provider  \u2610  Buyer shall be responsible for all taxes in connection with the purchase of Services in this Agreement.",
		template.FontSize(9),
	)
	spacer(p1, 3)

	// Section 3: Payment
	para(p1, "3. Payment. Payment for the Services will be by: (Check one)", template.FontSize(9))
	spacer(p1, 2)
	checkboxRow(p1, "Cash", "Credit or debit card")
	checkboxRow(p1, "Personal check", "Wire transfer")
	checkboxRow(p1, "Cashier's check", "Other: _______________")
	checkboxRow(p1, "Money order", "")

	if d.Notes != "" {
		spacer(p1, 4)
		para(p1, d.Notes, template.FontSize(9), template.TextColor(cGray))
	}

	// ══════════════════════════════════════════════════════════════════════════
	// PAGE 2 — PAYMENT PLAN
	// ══════════════════════════════════════════════════════════════════════════
	p2 := doc.AddPage()

	// Title
	p2.AutoRow(func(r *template.RowBuilder) {
		r.Col(12, func(c *template.ColBuilder) {
			c.Text("PAYMENT PLAN",
				template.FontSize(14),
				template.Bold(),
				template.Align(document.AlignCenter),
			)
		})
	})
	dividerRow(p2, 0.5, cBlack)
	spacer(p2, 4)

	// Contract text
	para(p2, fmt.Sprintf(
		`By this contract, %s agrees to make payments to %s, hereafter known as "Lender," by the following schedule in exchange for %s. This payment schedule is enforceable by law, and the methods described below will be used in cases of delinquent payment.`,
		d.Payer, d.Payee, d.Product),
		template.FontSize(9),
	)
	spacer(p2, 3)

	para(p2, fmt.Sprintf(
		`By this agreement, it is agreed that a payment of %s will be surrendered to the Lender every %s until the total of the payment required, which is %s, has been delivered. The payment plan will take the following form:`,
		d.AmountPerPeriod, d.Interval, d.TotalAmount),
		template.FontSize(9),
	)
	spacer(p2, 2)

	// Payment schedule table
	tableHeader(p2, []colDef{
		{"Payment Date", 6},
		{"Amount", 6},
	})

	for i, pmt := range d.Payments {
		bg := cWhite
		if i%2 != 0 {
			bg = cLightGray
		}
		tableDataRow(p2, []cellDef{
			{pmt.Date, 6, document.AlignCenter},
			{pmt.Amount, 6, document.AlignCenter},
		}, bg)
	}

	spacer(p2, 4)

	para(p2, "These payments include any interest and other charges that may apply.", template.FontSize(9))
	spacer(p2, 3)

	para(p2, fmt.Sprintf(
		`This agreement is binding, and failure to meet its terms will allow the Lender to take certain recourse. First, late payments will incur a fee of %s every %s. Insufficient payment and bounced checks will incur a fee of %s. If payment should not be delivered at all, Lender will be entitled to %s.`,
		d.LateFee, d.Interval, d.BounceFee, d.LenderAction),
		template.FontSize(9),
	)
	spacer(p2, 3)

	para(p2, "In addition, the following terms and conditions apply: "+d.TermsConditions, template.FontSize(9))
	spacer(p2, 3)

	para(p2,
		`By signing this agreement, all parties agree to the terms as described above. Alterations to this agreement can only be made by both parties and must be placed in writing. Both parties will receive a printed copy of this agreement, and will be responsible for upholding its terms.`,
		template.FontSize(9),
	)
	spacer(p2, 8)

	// Signature blocks
	sigBlock(p2, d.Payer)
	spacer(p2, 4)
	sigBlock(p2, d.Payee)

	// ── Generate & save ───────────────────────────────────────────────────────
	data, err := doc.Generate()
	if err != nil {
		return fmt.Errorf("generate: %w", err)
	}
	return os.WriteFile(path, data, 0644)
}
