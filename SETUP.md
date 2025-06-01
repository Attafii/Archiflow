# Environment Setup

## Google OAuth2 Configuration

This application requires Google OAuth2 credentials to sync with Google Calendar.

### Setup Instructions:

1. **Get Google OAuth2 Credentials:**
   - Go to [Google Cloud Console](https://console.cloud.google.com/)
   - Create a new project or select an existing one
   - Enable the Google Calendar API
   - Go to "Credentials" → "Create Credentials" → "OAuth client ID"
   - Choose "Desktop application"
   - Note down your Client ID and Client Secret

2. **Set Environment Variables:**
   
   **Option A: Using System Environment Variables (Recommended)**
   - On Windows: 
     ```cmd
     setx GOOGLE_CLIENT_ID "your-actual-client-id"
     setx GOOGLE_CLIENT_SECRET "your-actual-client-secret"
     ```
   - Restart your IDE/terminal after setting these

   **Option B: Using .env file (Development only)**
   - Copy `.env.example` to `.env`
   - Replace the placeholder values with your actual credentials
   - The .env file is ignored by Git for security

3. **Restart your application** after setting the environment variables.

### Security Notes:
- Never commit actual credentials to version control
- The .env file is excluded from Git via .gitignore
- Use environment variables in production environments
