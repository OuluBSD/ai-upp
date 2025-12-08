#!/bin/bash
# CI/CD script for uppts library

set -e  # Exit immediately if a command exits with a non-zero status

echo "Starting uppts CI/CD pipeline..."

# Check if we're in the right directory
if [ ! -f "package.json" ]; then
  echo "Error: package.json not found. Please run this script from the ts directory."
  exit 1
fi

echo "Node version: $(node --version)"
echo "NPM version: $(npm --version)"

# Install dependencies
echo "Installing dependencies..."
npm ci

# Run linter
echo "Running linter..."
npm run lint

# Run type checking
echo "Running type checking..."
npx tsc --noEmit
if [ $? -ne 0 ]; then
  echo "Type checking failed!"
  exit 1
fi

# Run tests with coverage
echo "Running tests with coverage..."
npm test -- --coverage

# Check coverage thresholds
echo "Checking coverage thresholds..."
npx jest --coverage --coverageThreshold='{"global":{"branches":60,"functions":75,"lines":70,"statements":70}}'

# Build the project
echo "Building project..."
npm run build

echo "All CI/CD checks passed!"