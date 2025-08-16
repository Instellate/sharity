defmodule Sharity.WsServer do
  def init(opts) do
    receiver = Keyword.get(opts, :receiver)
    :timer.send_interval(30_000, :ping)

    if receiver !== nil do
      msg =
        Jason.encode!(%{
          "type" => "connected"
        })

      send(receiver, {:connected, self()})
      Process.monitor(receiver)
      {:push, {:text, msg}, %{receiver: receiver}}
    else
      {:ok, %{}}
    end
  end

  def handle_in({message, [opcode: opcode]}, state) do
    if Map.has_key?(state, :receiver) do
      send(state.receiver, {:send, {opcode, message}})
    end

    {:ok, state}
  end

  def handle_info({:send, message}, state) do
    {:push, message, state}
  end

  def handle_info({:connected, receiver}, state) do
    msg =
      Jason.encode!(%{
        "type" => "connected"
      })

    Process.monitor(receiver)
    state = Map.put(state, :receiver, receiver)
    {:push, {:text, msg}, state}
  end

  def handle_info(:ping, state) do
    {:push, {:ping, "ping"}, state}
  end

  def handle_info({:DOWN, _ref, :process, _receiver, _reason}, state) do
    {:stop, :normal, state}
  end
end
