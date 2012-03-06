def createCppString(text):
    for c in text:
        out = c
        #escape escape characters
        if (c == '\\'):
            out = '\\\\'
        #replace line feeds with string continuation
        if (c == '\n'):
            out = '\\n" \\\n"'
        #escape explicit quotes
        if (c == '"'):
            out = '\\"'
        sys.stdout.write(out)
