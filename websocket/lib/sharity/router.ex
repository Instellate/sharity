defmodule Sharity.Router do
  use Plug.Router
  require Logger

  plug(Plug.Logger)
  plug(:match)
  plug(:dispatch)

  get "/" do
    send_resp(conn, 200, "Hello, world!")
  end

  get "/ws" do
    conn = fetch_query_params(conn)

    with :ok <- WebSockAdapter.UpgradeValidation.validate_upgrade(conn),
         %{"key" => encoded_key, "type" => type} <- conn.query_params,
         {:ok, public_key} <- Base.url_decode64(encoded_key, padding: false),
         32 <- byte_size(public_key) do
      case type do
        "uploader" ->
          handle_uploader(conn, public_key)

        "downloader" ->
          handle_downloader(conn, public_key)

        _ ->
          put_resp_content_type(conn, "application/json")
          send_resp(conn, 400, Jason.encode!(%{"message" => "Invalid type query param"}))
      end
    else
      {:error, msg} ->
        put_resp_content_type(conn, "application/json")
        send_resp(conn, 400, Jason.encode!(%{"message" => msg}))

      :error ->
        put_resp_content_type(conn, "application/json")
        send_resp(conn, 400, Jason.encode!(%{"message" => "Invalid base64"}))

      v when is_integer(v) ->
        put_resp_content_type(conn, "application/json")
        send_resp(conn, 400, Jason.encode!(%{"message" => "Invalid key size"}))

      _ ->
        put_resp_content_type(conn, "application/json")
        send_resp(conn, 400, Jason.encode!(%{"message" => "Invalid params"}))
    end
  end

  @spec handle_uploader(Plug.Conn.t(), binary()) :: term()
  defp handle_uploader(conn, public_key) do
    with {:ok, true} <- Sharity.Downloaders.create(public_key, self()) do
      WebSockAdapter.upgrade(conn, Sharity.WsServer, [], timeout: 60_000)
      |> halt()
    else
      {:ok, false} ->
        put_resp_content_type(conn, "application/json")
        send_resp(conn, 409, Jason.encode!(%{"message" => "Conflict"}))
    end
  end

  @spec handle_downloader(Plug.Conn.t(), binary()) :: term()
  defp handle_downloader(conn, public_key) do
    case Sharity.Downloaders.update(public_key) do
      {:ok, pid} ->
        IO.inspect(pid)
        WebSockAdapter.upgrade(conn, Sharity.WsServer, [receiver: pid], timeout: 60_000)
        |> halt()

      {:error, :not_found} ->
        put_resp_content_type(conn, "application/json")
        send_resp(conn, 404, Jason.encode!(%{"message" => "Not found"}))
    end
  end
end
