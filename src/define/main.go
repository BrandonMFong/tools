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

type BFError struct {
	message string
}

func (err *BFError) Error() string {
	return err.message
}

func main() {
	Define("hello")
}

func Define(word string) {
	data, err := GetDefinitionData(word)
	if err != nil {
		fmt.Println("error: ", err)
	} else {
		fmt.Println("word: ", word)
		DictionaryAPIRawJsonPrintMeanings(data)
	}
}

func DictionaryAPIRawJsonPrintMeanings(data []map[string]interface{}) {
	ent := data[0]
	fmt.Println(ent["meanings"])
	meanings := ent["meanings"]
	fmt.Println(meanings)
}

/**
creates dictionary entry
*/
func GetDefinitionData(word string) ([]map[string]interface{}, error) {
	jsonData := DictionaryAPIRawJsonFetchData(word)
	var data []map[string]interface{}
	err := json.Unmarshal([]byte(jsonData), &data)
	if err != nil {
		fmt.Printf("could not unmarshal json: %s\n", err)
		return data, &BFError{message: "could not parse dictionary data"}
	}

	return data, nil
}

/**
calls the dictionaryapi.dev api for word
*/
func DictionaryAPIRawJsonFetchData(word string) string {
	// Define the URL
    url := fmt.Sprintf("https://api.dictionaryapi.dev/api/v2/entries/en/%s", word)

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

