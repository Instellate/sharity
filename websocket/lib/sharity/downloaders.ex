defmodule Sharity.Downloaders do
  alias :ets, as: ETS
  use GenServer

  @type entry() :: {pid(), boolean()}

  def start_link(opts \\ []) do
    GenServer.start_link(__MODULE__, opts, name: __MODULE__)
  end

  @spec get(binary()) :: {:ok, entry()} | {:error, :not_found}
  def get(public_key) do
    GenServer.call(__MODULE__, {:get, public_key})
  end

  @spec create(binary(), pid()) :: {:ok, boolean()}
  def create(public_key, pid) do
    GenServer.call(__MODULE__, {:create, public_key, pid})
  end

  @spec update(binary()) :: {:ok, pid()} | {:error, :not_found}
  def update(public_key) do
    GenServer.call(__MODULE__, {:update, public_key})
  end

  @impl true
  def init(_opts) do
    ets = ETS.new(__MODULE__, [:set, :protected])

    {:ok, ets}
  end

  @impl true
  def handle_call({:get, public_key}, _from, state) do
    case ETS.lookup(state, public_key) do
      [{_, pid, connected}] -> {:reply, {:ok, pid, connected}, state}
      _ -> {:reply, {:error, :not_found}, state}
    end
  end

  @impl true
  def handle_call({:create, public_key, pid}, _from, state) do
    inserted = ETS.insert_new(state, {public_key, pid, false})

    if inserted do
      Process.monitor(pid)
    end

    {:reply, {:ok, inserted}, state}
  end

  @impl true
  def handle_call({:update, public_key}, _from, state) do
    reply =
      case ETS.lookup(state, public_key) do
        [{_, pid, false}] ->
          ETS.update_element(state, public_key, {3, true})
          {:ok, pid}
        _ ->
          {:error, :not_found}
      end

    {:reply, reply, state}
  end

  @impl true
  def handle_info({:DOWN, _ref, :process, proc, _reason}, state) do
    ETS.match_delete(state, {:_, proc, :_})
    {:noreply, state}
  end
end
