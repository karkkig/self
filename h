func (s *StockService) AddStock(symbol string, name string) error {

	url := fmt.Sprintf(
		"https://www.alphavantage.co/query?function=TIME_SERIES_DAILY&symbol=%s&apikey=%s",
		symbol,
		s.AlphaKey,
	)

	client := http.Client{Timeout: 10 * time.Second}
	resp, err := client.Get(url)
	if err != nil {
		return err
	}
	defer resp.Body.Close()

	body, err := io.ReadAll(resp.Body)
	if err != nil {
		return err
	}

	// 🔍 Rate limit check
	var check map[string]interface{}
	_ = json.Unmarshal(body, &check)

	if msg, ok := check["Note"]; ok {
		return fmt.Errorf("alphavantage rate limit: %v", msg)
	}
	if msg, ok := check["Information"]; ok {
		return fmt.Errorf("alphavantage rate limit: %v", msg)
	}

	// 📊 Parse response
	type TimeSeriesResponse struct {
		TimeSeries map[string]map[string]string `json:"Time Series (Daily)"`
	}

	var data TimeSeriesResponse
	if err := json.Unmarshal(body, &data); err != nil {
		return err
	}

	if data.TimeSeries == nil {
		return fmt.Errorf("no data for %s", symbol)
	}

	// 🧠 Find latest price
	var latestDate time.Time
	var latestPrice float64

	for dateStr, values := range data.TimeSeries {

		date, err := time.Parse("2006-01-02", dateStr)
		if err != nil {
			continue
		}

		price, err := strconv.ParseFloat(values["4. close"], 64)
		if err != nil {
			continue
		}

		// Save history
		if err := s.Repo.UpdateHistoryBySymbol(symbol, price, date); err != nil {
			fmt.Println("History insert error:", err)
		}

		if date.After(latestDate) {
			latestDate = date
			latestPrice = price
		}
	}

	if latestPrice == 0 {
		return fmt.Errorf("failed to get latest price")
	}

	// 💾 Save stock
	stock := &models.Stock{
		Symbol:    symbol,
		StockName: name,
		LastPrice: latestPrice,
	}

	if err := s.Repo.Save(stock); err != nil {
		return err
	}

	// 🔄 Update latest price
	savedStock, err := s.Repo.GetBySymbol(symbol)
	if err != nil {
		return err
	}

	return s.Repo.UpdateStockPrice(savedStock.ID, latestPrice)
}
