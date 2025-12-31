let () =
  Dream.run
  @@ Dream.logger
  @@ Dream.router [
    Dream.get "/" (fun _ ->
      Dream.html "Welcome to the Crypto Trader Dashboard!");
    
    Dream.get "/status" (fun _ ->
      Dream.json "{\"status\": \"running\", \"active_traders\": 0}");

    (* SKELETON: Performance updates from C++ trader *)
    Dream.post "/update/performance" (fun request ->
      let%lwt body = Dream.body request in
      Dream.log "Received performance update: %s" body;
      Dream.empty `OK);

    (* SKELETON: Start a new trader instance *)
    Dream.post "/trader/start" (fun request ->
      let%lwt _body = Dream.body request in
      (* Logic to spin up C++ process would go here *)
      Dream.json "{\"status\": \"starting\", \"id\": \"instance_1\"}");

    (* SKELETON: Configure a trader *)
    Dream.post "/trader/configure" (fun _ ->
      Dream.json "{\"status\": \"configured\"}");
  ]