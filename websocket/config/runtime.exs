import Config

if config_env() === :prod do
  config :sharity,
    stun_servers: System.get_env("STUN_SERVERS") |> String.split(" ")
end
