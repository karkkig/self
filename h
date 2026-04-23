badgeData := pdf.GPayBadgeData{
		BusinessName: "Your Business Name",
		PhoneNumber:  "+91 12345 67890",
		UPIHandle:    "12345 67890@yhh",
		QRCodePath:   "", // Set to your QR image path, e.g. "qr.png"
	}
	if err := pdf.GenerateGPayBadge("gpay_badge.pdf", badgeData); err != nil {
		log.Fatal("gpay badge:", err)
	}
