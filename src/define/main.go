/*
 * author: brando
 * date: 5/22/24
 */
package main

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
    "net/http"
)

func main() {
	define()
}

func define() {
	jsonData := fetchDefinition()
	var data []map[string]interface{}
	err := json.Unmarshal([]byte(jsonData), &data)
	if err != nil {
		fmt.Printf("could not unmarshal json: %s\n", err)
		return
	}

	fmt.Printf("json map: %v\n", data)

}

func fetchDefinition() string {
	// Define the URL
    url := "https://api.dictionaryapi.dev/api/v2/entries/en/hello"

    // Make the HTTP GET request
    resp, err := http.Get(url)

    // Handle errors
    if err != nil {
        fmt.Println("Error fetching URL:", err)
        return ""
    }

    // Close the response body after use
    defer resp.Body.Close()

    // Check the status code
    if resp.StatusCode != http.StatusOK {
        fmt.Println("Error:", resp.StatusCode)
        return ""
    }

    // Read the content of the response body
    body, err := ioutil.ReadAll(resp.Body)
    if err != nil {
        fmt.Println("Error reading response body:", err)
        return ""
    }

    // Optionally, parse the content based on its format (JSON, XML, etc.)
	return string(body)

}
