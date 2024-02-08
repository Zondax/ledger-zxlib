# Python script to generate the yearLookup array for C code, covering years up to 2600

def generate_year_lookup():
    year = 1970
    days = 0
    year_days = 365  # Most years have 365 days
    yearLookup_code = []

    # Loop until we reach the year 2500
    while year <= 2600:
        # Check if it's a leap year
        if (year % 4 == 0 and year % 100 != 0) or (year % 400 == 0):
            year_days = 366
        else:
            year_days = 365

        # Add the days of the current year to the cumulative total
        days += year_days
        # Append the cumulative days to the yearLookup array
        yearLookup_code.append(days)

        # Move to the next year
        year += 1

    # Generating C code snippet for the array
    year_lookup_c_snippet = "const uint32_t yearLookup[] = {\n    " + ",\n    ".join(map(str, yearLookup_code[:-1])) + "\n};"
    return year_lookup_c_snippet

# Print the C code snippet
print(generate_year_lookup())
