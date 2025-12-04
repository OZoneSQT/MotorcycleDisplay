- [X] Verify that the copilot-instructions.md file in the .github directory is created.

- [X] Clarify Project Requirements  
	Ask for project type, language, and frameworks if not specified. Skip if already provided.

- [X] Scaffold the Project  
	Ensure that the previous step has been marked as completed.
	Sanity check of project
	Call project setup tool with projectType parameter.  
	Run scaffolding command to create project files and folders.  
	Use '.' as the working directory.  
	If no appropriate projectType is available, search documentation using available tools.  
	Otherwise, create the project structure manually using available file creation tools.  
    Sanity check of project

- [X] Customize the Project  
	Verify that all previous steps have been completed successfully and you have marked the step as completed.  
	Develop a plan to modify codebase according to user requirements.  
	Apply modifications using appropriate tools and user-provided references.  
	Skip this step for "Hello World" projects.  

- [X] Install Required Extensions  
	ONLY install extensions provided mentioned in the get_project_setup_info. Skip this step otherwise and mark as completed.

- [X] Compile the Project  
	Verify that all previous steps have been completed.  
	Install any missing dependencies.  
	Run diagnostics and resolve any issues.  
	Check for markdown files in project folder for relevant instructions on how to do this.  

- [X] Create and Run Task  
	Verify that all previous steps have been completed.  
	Check https://code.visualstudio.com/docs/debugtest/tasks to determine if the project needs a task. If so, use the create_and_run_task to create and launch a task based on package.json, README.md, and project structure.  
	Skip this step otherwise.

- [X] Launch the Project   
	Verify that all previous steps have been completed.  
	Prompt user for debug mode, launch only if confirmed.  

- [X] Ensure Documentation is Complete  
	Verify that all previous steps have been completed.  
	Verify that README.md and the copilot-instructions.md file in the .github directory exist and contain current project information.  
	README.md must include:  
	- Project title and description  
	- Setup instructions  
	- Usage examples  
	- Testing instructions  
	- Logging and error handling details (including `.csv` format)  
	- Security considerations
	- Spelling and grammar check for clarity and professionalism.
	- Add Dependencies and installation instructions	
	- Add Contribution guidelines
	- Spelling and grammar check for clarity and professionalism.

---

## UX Guidelines

Prioritize user experience in all code changes.

- Provide clear and informative feedback to users.
- Provide intuitive navigation and interaction flows if applicable.
- ensure process status visibility in terminal for users during long-running operations, unless running -Silent mode.

---

## Clean Code Standards

All code must follow the principles from *Clean Code* by Robert C. Martin:

- Use **descriptive, intention-revealing names** for all identifiers.  
- Keep **functions small and focused** on a single task.  
- Apply the **Single Responsibility Principle** to all classes and modules.  
- Avoid code duplication (DRY).  
- Write **self-explanatory code**; use comments only to explain *why*, not *what*.  
- Maintain **consistent formatting** and structure.  
- Validate inputs early and **fail fast** with meaningful errors.

---

## Clean Architecture Guidelines

Structure the project using Clean Architecture:

- **Entities**: Core business rules and logic.  
- **Use Cases**: Application-specific orchestration.  
- **Interface Adapters**: Controllers, presenters, gateways.  
- **Frameworks & Drivers**: UI, database, external services.

**Dependency Rule**: Inner layers must not depend on outer layers. Use interfaces and inversion of control.

---

## Secure Coding Requirements

Security is mandatory unless explicitly waived:

- Sanitize and validate all external inputs.  
- Never hardcode secrets—use environment variables or secure vaults.  
- Apply the **principle of least privilege**.  
- Avoid exposing sensitive data in logs or errors.  
- Protect against common vulnerabilities (e.g., XSS, SQL injection, CSRF).

---
Checks: '-*,readability-identifier-naming'
WarningsAsErrors: 'readability-identifier-naming'
CheckOptions:
	- key: readability-identifier-naming.VariablePrefix
		value: 'psz|p|i|u|b|f|dw|c|n'
	- key: readability-identifier-naming.MemberPrefix
		value: 'm_'
	- key: readability-identifier-naming.FunctionCase
		value: camelBack
	- key: readability-identifier-naming.VariableCase
		value: camelBack
	- key: readability-identifier-naming.ConstantCase
		value: UPPER_CASE
...

## Unit Testing Policy

Unit tests are required for all business logic:

- Target >80% coverage on core logic.  
- Tests must be isolated and deterministic.  
- Use descriptive test names: `should_<behavior>_when_<condition>`.  
- Include tests for edge cases and failure scenarios.

---

## Error Logging Standards

All runtime errors must be logged:

- Use structured logging (e.g., `.csv` format).  
- Apply appropriate log levels (INFO, WARN, ERROR).  
- Never log sensitive data (e.g., passwords, tokens).  
- Integrate with centralized logging platforms if applicable.

---

## Copilot Code Review Integration

Copilot must perform an automated code review before any code is finalized or merged. This review should include:

- Clean Code Compliance, suggest improvements if necessary
- Architecture Validation, suggest improvements if necessary
- Hungarian Notation Compliance, suggest improvements if necessary
- Security Checks, suggest improvements if necessary
- Unit Test Coverage, suggest improvements if necessary
- Error Logging Verification, suggest improvements if necessary
- Documentation Completeness, suggest improvements if necessary

---

## Final Checklist

- [ ] Sanity check of project
- [ ] Clean Code principles applied  
- [ ] Clean Architecture structure enforced  
- [ ] Secure coding practices implemented  
- [ ] Inputs are validated and sanitized
- [ ] Hungarian Notation followed
- [ ] Unit tests cover >80% of business logic  
- [ ] Unit tests written and passing  
- [ ] Errors are logged to `.csv` with proper structure  
- [ ] No hardcoded secrets or sensitive data in logs  
- [ ] README.md is updated with setup, usage, testing, logging, and security info


# copilot-instructions.md — PSInstaller (concise agent guide)

Purpose: provide immediate, actionable guidance for AI coding agents working in this repository — what to edit, how to run things, and where to look for conventions.

- Descripe Big Picture
- Developer workflows (exact commands)
- Install dependicies / setup script
- Quick tests
- Project-specific conventions & patterns
- Logging format
- Update documentation, — mermaid diagrams, Class diagrams and flow details.
- Refractor
- Edit & test checklist (short)
- Run Unit tests

Clean up the copilot-instructions.md file in the .github directory by removing all HTML comments.
