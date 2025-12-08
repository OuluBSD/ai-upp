# CI/CD Pipeline Configuration

This directory contains configurations for the Continuous Integration and Continuous Deployment pipeline for the uppts library.

## GitHub Actions Workflow

The main CI/CD pipeline is defined in `.github/workflows/ci-cd.yml` and includes:

- Automated testing on multiple Node.js versions
- Code linting and type checking
- Test coverage reporting
- Security auditing
- Build verification

## Local CI/CD Script

A local script `scripts/ci-cd-check.sh` is provided that runs the same checks as the CI/CD pipeline. This is useful for:

- Pre-commit validation
- Local testing before pushing
- Debugging CI/CD failures

To run the local CI/CD checks:

```bash
cd ts
./scripts/ci-cd-check.sh
```

## Coverage Requirements

The CI/CD pipeline enforces minimum coverage thresholds:
- Statements: 70%
- Branches: 60%
- Functions: 75%
- Lines: 70%

## Security

The pipeline includes security auditing using `npm audit` to detect known vulnerabilities in dependencies.

## Artifacts

Successful builds upload the compiled output as build artifacts that can be downloaded and used for releases.