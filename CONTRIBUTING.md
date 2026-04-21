# Contributing to libarm64emu

Thank you for considering contributing to libarm64emu! This document provides guidelines and instructions for contributors.

## Code of Conduct

Be respectful, inclusive, and constructive. We welcome contributors of all backgrounds and experience levels.

## How to Contribute

### Reporting Bugs

1. Check existing issues first
2. Use the bug report template
3. Include:
   - Platform (OS, compiler version)
   - Steps to reproduce
   - Expected vs actual behavior
   - Minimal code example if possible

### Suggesting Features

1. Open a discussion first (not an issue)
2. Explain the use case
3. Describe the proposed solution
4. Be open to feedback and alternatives

### Pull Requests

1. Fork the repository
2. Create a branch (`git checkout -b feature/my-feature`)
3. Make your changes
4. Add/update tests
5. Ensure tests pass
6. Update documentation
7. Submit PR with clear description

## Coding Standards

### C++ Style

- **Standard:** C++17
- **Formatting:** Consistent with existing code (we'll add clang-format config)
- **Naming:**
  - Types: `PascalCase` (e.g., `SystemRegisterEmulator`)
  - Functions: `camelCase` (e.g., `emulateMSR`)
  - Variables: `camelCase` with trailing `_` for members (e.g., `state_`)
  - Constants: `UPPER_CASE` (e.g., `RESET_VALUE`)
  - Namespaces: `lowercase` (e.g., `arm64emu`)

### Documentation

- All public APIs must have doc comments
- Use Doxygen-style comments:
```cpp
/// @brief Brief description
/// @param param1 Description of parameter
/// @return Description of return value
```

### Testing

- All new features require tests
- Bug fixes should include regression tests
- Aim for >80% code coverage
- Run tests before submitting PR: `ctest --output-on-failure`

## Architecture Guidelines

### What Belongs in libarm64emu

✅ Generic ARM64 system register emulation
✅ Standard ARM architecture features (per ARM ARM)
✅ Hypervisor-agnostic code
✅ Well-documented, tested implementations

### What Does NOT Belong

❌ Apple-specific code (AVPBooter, iBoot, etc.)
❌ iOS circumvention techniques
❌ Platform-specific hypervisor code (belongs in backend)
❌ Undocumented/vendor-specific registers
❌ Exploit code or security bypasses

## Review Process

1. Maintainer reviews code quality
2. Check tests pass
3. Verify documentation is complete
4. Ensure architecture guidelines followed
5. Merge after approval

## Release Process

Releases follow semantic versioning (MAJOR.MINOR.PATCH):

- **PATCH:** Bug fixes (backward compatible)
- **MINOR:** New features (backward compatible)
- **MAJOR:** Breaking changes

## Questions?

Open a discussion or contact maintainers.

---

**By contributing, you agree to license your contributions under the MIT License.**
