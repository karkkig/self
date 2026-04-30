package main

import (
	"fmt"
	"log"

	"yourmodule/mapdf"
)

func main() {
	data := mapdf.FullAgreementData{
		// ── Page 1: Services Agreement ────────────────────────────────────────
		State:           "California",
		Day:             "1st",
		Month:           "January",
		Year:            "25",
		ProviderName:    "Acme Services Inc.",
		ProviderAddress: "123 Main Street, Los Angeles, CA 90001",
		BuyerName:       "Globex Corporation",
		BuyerAddress:    "456 Market Street, San Francisco, CA 94105",
		Services: []mapdf.ServiceItem{
			{Description: "Web Development", NumProjects: "2", PricePerProject: "5,000"},
			{Description: "UI/UX Design", NumProjects: "1", PricePerProject: "3,000"},
			{Description: "SEO Optimization", NumProjects: "3", PricePerProject: "1,200"},
			{Description: "Monthly Maintenance", NumProjects: "6", PricePerProject: "800"},
		},
		PurchasePrice: "20,000",
		Notes:         "All deliverables will be provided digitally. Revisions are limited to 3 rounds per project.",

		// ── Page 2: Payment Plan ──────────────────────────────────────────────
		Payer:           "Globex Corporation",
		Payee:           "Acme Services Inc.",
		Product:         "Web Development & Design Services",
		AmountPerPeriod: "$3,334",
		Interval:        "month",
		TotalAmount:     "$10,000",
		Payments: []mapdf.PaymentEntry{
			{Date: "1 February 2025", Amount: "$3,334"},
			{Date: "1 March 2025", Amount: "$3,334"},
			{Date: "1 April 2025", Amount: "$3,332"},
		},
		LateFee:         "$50",
		BounceFee:       "$75",
		LenderAction:    "engage a debt collection service and pursue legal remedies",
		TermsConditions: "No refunds after commencement of work. All disputes are subject to California jurisdiction.",
	}

	out := "full_agreement.pdf"
	if err := mapdf.GenerateFullAgreement(out, data); err != nil {
		log.Fatalf("error: %v", err)
	}
	fmt.Println("✅ full_agreement.pdf generated")
}package main

import (
	"fmt"
	"log"

	"yourmodule/mapdf"
)

func main() {
	data := mapdf.FullAgreementData{
		// ── Page 1: Services Agreement ────────────────────────────────────────
		State:           "California",
		Day:             "1st",
		Month:           "January",
		Year:            "25",
		ProviderName:    "Acme Services Inc.",
		ProviderAddress: "123 Main Street, Los Angeles, CA 90001",
		BuyerName:       "Globex Corporation",
		BuyerAddress:    "456 Market Street, San Francisco, CA 94105",
		Services: []mapdf.ServiceItem{
			{Description: "Web Development", NumProjects: "2", PricePerProject: "5,000"},
			{Description: "UI/UX Design", NumProjects: "1", PricePerProject: "3,000"},
			{Description: "SEO Optimization", NumProjects: "3", PricePerProject: "1,200"},
			{Description: "Monthly Maintenance", NumProjects: "6", PricePerProject: "800"},
		},
		PurchasePrice: "20,000",
		Notes:         "All deliverables will be provided digitally. Revisions are limited to 3 rounds per project.",

		// ── Page 2: Payment Plan ──────────────────────────────────────────────
		Payer:           "Globex Corporation",
		Payee:           "Acme Services Inc.",
		Product:         "Web Development & Design Services",
		AmountPerPeriod: "$3,334",
		Interval:        "month",
		TotalAmount:     "$10,000",
		Payments: []mapdf.PaymentEntry{
			{Date: "1 February 2025", Amount: "$3,334"},
			{Date: "1 March 2025", Amount: "$3,334"},
			{Date: "1 April 2025", Amount: "$3,332"},
		},
		LateFee:         "$50",
		BounceFee:       "$75",
		LenderAction:    "engage a debt collection service and pursue legal remedies",
		TermsConditions: "No refunds after commencement of work. All disputes are subject to California jurisdiction.",
	}

	out := "full_agreement.pdf"
	if err := mapdf.GenerateFullAgreement(out, data); err != nil {
		log.Fatalf("error: %v", err)
	}
	fmt.Println("✅ full_agreement.pdf generated")
}
