defmodule Sharity.Application do
  use Application
  require Logger

  def start(_type, _args) do
    children = [
      {Bandit, plug: Sharity.Router, port: Application.fetch_env!(:sharity, :port)},
      Sharity.Downloaders
    ]

    Supervisor.start_link(children, strategy: :one_for_one, name: Sharity.Supervisor)
  end
end
