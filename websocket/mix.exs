defmodule Sharity.MixProject do
  use Mix.Project

  def project do
    [
      app: :sharity,
      version: "0.1.0",
      elixir: "~> 1.18",
      start_permanent: Mix.env() == :prod,
      deps: deps(),
      aliases: aliases()
    ]
  end

  # Run "mix help compile.app" to learn about applications.
  def application do
    [
      extra_applications: [:logger],
      mod: {Sharity.Application, []}
    ]
  end

  defp deps do
    [
      {:plug, "~> 1.18"},
      {:plug_cowboy, "~> 2.7"},
      {:websock_adapter, "~> 0.5.8"},
      {:bandit, "~> 1.7.0"},
      {:jason, "~> 1.4"}
    ]
  end

  defp aliases do
    [
      "server.run": ["run --no-halt"]
    ]
  end
end
