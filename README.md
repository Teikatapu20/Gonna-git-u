# GameHub Inventory Management System

A beginner-friendly C++ console program for managing a gaming shop's games, consoles, and accessories. It supports add, view, search, update, delete, and inventory summary. Data is stored in `products.csv` in the program's current working directory.

## Build and run

Requires a C++11-or-newer compiler. Open this folder in VS Code and compile from its terminal:

```bash
g++ -std=c++11 -Wall -Wextra -pedantic main.cpp Inventory.cpp Storage.cpp Input.cpp -o gamehub
./gamehub
```

On Windows, run `gamehub.exe` after compilation. The CSV file is created automatically when the program saves for the first time.

## Participant work allocation

| Participant | Responsibility | Suggested files and deliverables |
|---|---|---|
| Member 1: Repository coordinator | Create the repository, agree on file names, manage branches/merges, own menu and integration | `main.cpp`, project setup, integration review |
| Member 2: Core calculations / business logic and feature development | Implement inventory operations, duplicate-ID rule, search, stock valuation, and unit totals | `Inventory.h`, `Inventory.cpp` |
| Member 3: File storage, records, and input validation | Define product fields, load/save CSV, validate user input, preserve records between runs | `Product.h`, `Storage.h`, `Storage.cpp`, `Input.h`, `Input.cpp` |
| Member 4: Additional feature, test coordination, and quality review | Own and review the low-stock alert in the summary, coordinate the manual checks, and document issues; optional extension: sort products by name or export low-stock items | `main.cpp` summary presentation, review checklist, optional feature branch |

### Collaboration sequence

1. Member 1 creates the repository and adds the starter files and shared interfaces (`Product.h`, `Inventory.h`, and `Storage.h`).
2. Members 2 and 3 work on separate branches using those agreed interfaces.
3. Member 4 prepares the manual test checklist and reviews pull requests for duplicate IDs, invalid values, persistence, and menu behavior.
4. Member 1 integrates the branches, resolves conflicts, and makes sure the complete program builds before the group submits it.

## Manual review checklist

- Add a game, a console, and an accessory; confirm each appears in View all.
- Try adding an existing ID; confirm the duplicate is rejected.
- Search by part of the name, category, platform, and ID.
- Update price, quantity, and reorder threshold; leave some fields blank to keep their values.
- Delete an existing product and cancel another deletion.
- Confirm the summary totals units and value and identifies items at or below their reorder thresholds.
- Close and reopen the program; confirm saved products remain available.
- Enter invalid menu numbers, negative quantities, invalid prices, and blank required fields.

## Current scope

This is an educational stock-record tool. It tracks item counts and estimated stock value; it does not process sales, payments, taxes, or supplier orders.
