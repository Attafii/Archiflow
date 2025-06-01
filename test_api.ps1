$headers = @{
    "Content-Type" = "application/json"
    "Authorization" = "Bearer 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"
}

$body = @{
    "model" = "mixtral-8x7b-32768"
    "messages" = @(
        @{
            "role" = "user"
            "content" = "You are an intelligent assistant for a contract management application. IMPORTANT: Always respond with ONLY valid JSON format, no additional text or markdown. Analyze the following request: 'hi'. If it's a specific contract modification request (payment terms or non-compete clause), respond with JSON: {'type': 'payment_terms', 'value': NUMBER} or {'type': 'non_compete_clause'}. If it's general conversation, greeting, or question, respond with JSON: {'type': 'general', 'response': 'your conversational response'}. Examples: For 'Hello': {'type': 'general', 'response': 'Hello! How can I help you with your contracts today?'} For 'modify payment terms to 30 days': {'type': 'payment_terms', 'value': 30} For 'How are you?': {'type': 'general', 'response': 'I am doing well, thank you! How can I assist you?'} RESPOND WITH ONLY THE JSON OBJECT, NO OTHER TEXT."
        }
    )
    "temperature" = 0.3
    "max_tokens" = 150
    "top_p" = 1.0
    "stream" = $false
} | ConvertTo-Json -Depth 10

try {
    $response = Invoke-RestMethod -Uri "https://api.groq.com/openai/v1/chat/completions" -Method Post -Headers $headers -Body $body
    Write-Host "SUCCESS: API Response received"
    Write-Host "Full Response:" $response
    Write-Host "Content:" $response.choices[0].message.content
} catch {
    Write-Host "ERROR:" $_.Exception.Message
    Write-Host "Response:" $_.Exception.Response
}
