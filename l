package main

import (
	"fmt"
	"os"
	"strings"

	gpdf "github.com/gpdf-dev/gpdf"
	"github.com/gpdf-dev/gpdf/document"
	"github.com/gpdf-dev/gpdf/template"
)

func main() {
	doc := gpdf.NewDocument(
		gpdf.WithPageSize(document.A4),
		gpdf.WithMargins(document.UniformEdges(document.Mm(20))),
	)

	page := doc.AddPage()

	paragraph := strings.Join([]string{
		"Lorem ipsum dolor sit amet, consectetur adipiscing elit.",
		"Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.",
		"Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.",
		"Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur.",
		"Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.",
	}, " ")

	page.AutoRow(func(r *template.RowBuilder) {
		r.Col(12, func(c *template.ColBuilder) {
			c.Text("Random Paragraph Example",
				template.FontSize(18),
				template.Bold(),
			)
		})
	})

	page.AutoRow(func(r *template.RowBuilder) {
		r.Col(12, func(c *template.ColBuilder) {
			c.Text(paragraph,
				template.FontSize(11),
			)
		})
	})

	data, err := doc.Generate()
	if err != nil {
		panic(err)
	}

	if err := os.WriteFile("random_paragraph.pdf", data, 0644); err != nil {
		panic(err)
	}

	fmt.Println("PDF created: random_paragraph.pdf")
}
