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
	json.Unmarshal(body, &check)

	if msg, ok := check["Note"]; ok {
		return fmt.Errorf("alphavantage rate limit: %v", msg)
	}
	if msg, ok := check["Information"]; ok {
		return fmt.Errorf("alphavantage rate limit: %v", msg)
	}

	// 🔄 CHANGE STARTS HERE (parsing new API)

	var result map[string]map[string]map[string]string
	err = json.Unmarshal(body, &result)
	if err != nil {
		return err
	}

	timeSeries, ok := result["Time Series (Daily)"]
	if !ok || len(timeSeries) == 0 {
		return fmt.Errorf("no time series data for %s", symbol)
	}

	var latestDate time.Time
	var latestPrice float64

	for dateStr, values := range timeSeries {

		date, err := time.Parse("2006-01-02", dateStr)
		if err != nil {
			continue
		}

		price, err := strconv.ParseFloat(values["4. close"], 64)
		if err != nil {
			continue
		}

		// Keep only latest (same logic as before)
		if date.After(latestDate) {
			latestDate = date
			latestPrice = price
		}
	}

	if latestPrice == 0 {
		return fmt.Errorf("failed to extract latest price")
	}

	// 🔄 CHANGE ENDS HERE

	stock := &models.Stock{
		Symbol:    symbol,
		StockName: name,
		LastPrice: latestPrice,
	}

	err = s.Repo.Save(stock)
	if err != nil {
		return err
	}

	var stockID uint

	savedStock, err := s.Repo.GetBySymbol(symbol)
	if err != nil {
		return err
	}
	stockID = savedStock.ID

	// ✅ same repo call, just using new extracted values
	err = s.Repo.UpdateHistory(stockID, latestPrice, latestDate)
	if err != nil {
		return err
	}

	return nil
}
