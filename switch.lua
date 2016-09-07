-- Written by Pedro Martelletto in November 2010.
-- Updated to work with Postfix in April 2012.
-- Public domain.

rules = {
	{ pattern = "\nX%-OSBF%-Lua%-Score: %-%d+%.%d+/%d+%.%d+ %[S%]",
	  maildir = "/home/pedro/mail/spam" },
	{ pattern = "\nList%-Id: Lua mailing list <lua%-l%-lists%.lua%.org>",
	  maildir = "/home/pedro/mail/lua" },
}

header = ""
for line in io.lines() do
	if line == "" then break end -- end of header section
	header = header .. line .. "\n"
end

maildir = "/home/pedro/mail/default"
for _, rule in pairs(rules) do
	if string.find(header, rule.pattern) then
		maildir = rule.maildir
		break
	end
end

deliver = io.popen("/home/pedro/bin/deliver " .. maildir, "w")
if not deliver then
	io.stderr:write("could not fork\n")
	os.exit(71) -- EX_OSERR
end

deliver:write(header .. "\n") -- dump header + delimiter
for line in io.lines() do
	deliver:write(line .. "\n") -- and remaining data
end

os.exit(deliver:close())
