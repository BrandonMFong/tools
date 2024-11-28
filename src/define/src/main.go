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
	"os"
)

const ARG_BRIEF_DESCRIPTION = "--brief-description"

type BFError struct {
	message string
}

func (err *BFError) Error() string {
	return err.message
}

func help() {
	fmt.Printf("usage: %s <word>\n", os.Args[0])
	fmt.Println()
	fmt.Println("Copyright © 2024 Brando. All rights reserved.")
}

func BriefDescription() {
	fmt.Println("access dictionaryapi.dev to find definition for word")
}

func main() {
	word, briefdesc, err := ArgumentsRead()
	if err != nil {
		fmt.Println("error:", err)
		help()
		os.Exit(1)
	} else if briefdesc {
		BriefDescription();
		os.Exit(0)
	}

	err = Define(word)
	if err != nil {
		fmt.Println("error:", err)
		os.Exit(1)
	}

	os.Exit(0)
}

/**
reads arguments

returns: (word, brief description flag, error)
*/
func ArgumentsRead() (string, bool, error) {
	if len(os.Args) < 2 {
		return "", false, &BFError{message: "no arguments"}
	}
	
	arg := os.Args[len(os.Args) - 1]

	if arg == ARG_BRIEF_DESCRIPTION {
		return "", true, nil
	}

	return arg, false, nil
}

func Define(word string) error {
	data, err := GetDefinitionData(word)
	if err != nil {
		return err
	} else {
		fmt.Println("word: ", word)
		DictionaryAPIRawJsonPrintMeanings(data)
	}

	return nil
}

func DictionaryAPIRawJsonPrintMeanings(data []map[string]interface{}) {
	ent := data[0]
	meanings := ent["meanings"].([]interface{})

	fmt.Println("meanings:")
	for i, meaning := range meanings {
		m := meaning.(map[string]interface{})
		fmt.Printf("  %d : %s\n", i, m["partOfSpeech"])
		defs := m["definitions"].([]interface{})
		for _, def := range defs {
			d := def.(map[string]interface{})
			fmt.Println("      -", d["definition"])
		}
	}
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

