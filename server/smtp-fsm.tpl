AutoGen Definitions fsm;

state = parse_cmd, process_cmd, send_response;
event = cmd, cmd_ok, cmd_err, cmd_processed, quit, resp_sent;
type = reent;
method = case;
prefix = smtp;
cookie = "struct session *session";

transition = { tst = init; tev = cmd; next = parse_cmd; };
transition = { tst = init; tev = quit; next = done; };
transition = { tst = parse_cmd; tev = cmd_ok; next = process_cmd; };
transition = { tst = parse_cmd; tev = cmd_err; next = send_response; };
transition = { tst = process_cmd; tev = cmd_processed; next = send_response; };
transition = { tst = process_cmd; tev = quit; next = done; };
transition = { tst = send_response; tev = resp_sent; next = init; };
transition = { tst = send_response; tev = quit; next = done; };


