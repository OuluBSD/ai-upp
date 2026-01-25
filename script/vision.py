import os
import sys
import base64
import requests
import argparse
import json

def encode_image(image_path):
    with open(image_path, "rb") as image_file:
        return base64.b64encode(image_file.read()).decode('utf-8')

def main():
    parser = argparse.ArgumentParser(description="Analyze an image using OpenAI Vision API.")
    parser.add_argument("image_path", help="Path to the image file.")
    parser.add_argument("--prompt", required=True, help="Description of what to analyze in the image.")
    parser.add_argument("--require", nargs='+', required=True, help="List of keys expected to be true in the JSON response.")
    parser.add_argument("--api_key_path", default="~/openai-key.txt", help="Path to the API key file.")

    args = parser.parse_args()

    if not os.path.exists(args.image_path):
        print(f"Error: Image file '{args.image_path}' not found.")
        sys.exit(1)

    key_path = os.path.expanduser(args.api_key_path)
    if not os.path.exists(key_path):
        print(f"Error: API key file '{key_path}' not found.")
        sys.exit(1)

    with open(key_path, "r") as f:
        api_key = f.read().strip()

    base64_image = encode_image(args.image_path)

    headers = {
        "Content-Type": "application/json",
        "Authorization": f"Bearer {api_key}"
    }

    # Construct the requirements part of the prompt
    req_list = "\n".join([f'- \"{req}\": true/false' for req in args.require])
    
    final_prompt = (
        f"{args.prompt}\n\n"
        "Respond ONLY with a valid JSON object in the following format:\n"
        "{\n"
        f"{req_list},\n"
        "  \"description\": \"short summary\"\n"
        "}"
    )

    payload = {
        "model": "gpt-4o",
        "messages": [
            {
                "role": "user",
                "content": [
                    {
                        "type": "text",
                        "text": final_prompt
                    },
                    {
                        "type": "image_url",
                        "image_url": {
                            "url": f"data:image/png;base64,{base64_image}"
                        }
                    }
                ]
            }
        ],
        "max_tokens": 300
    }

    try:
        response = requests.post("https://api.openai.com/v1/chat/completions", headers=headers, json=payload)
        response.raise_for_status()
        result = response.json()
        content = result['choices'][0]['message']['content']
        print("AI Response:\n" + content)

        # Remove markdown code blocks if present
        content = content.replace("```json", "").replace("```", "").strip()
        
        try:
            data = json.loads(content)
            
            missing = []
            for req in args.require:
                if not data.get(req, False):
                    missing.append(req)
            
            if not missing:
                print("\nValidation: SUCCESS - All required elements detected.")
                sys.exit(0)
            else:
                print("\nValidation: FAILURE - Missing elements.")
                for req in args.require:
                    print(f"  - {req}: {'Found' if data.get(req, False) else 'Missing'}")
                sys.exit(1)
        except json.JSONDecodeError:
             print("Error: Failed to parse JSON response.")
             sys.exit(1)

    except requests.exceptions.RequestException as e:
        print(f"API Request Error: {e}")
        if response.content:
             print(f"Response content: {response.content.decode()}")
        sys.exit(1)
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()