FROM elixir:alpine AS build

COPY ./ /src
WORKDIR /src

ENV MIX_ENV=prod
RUN mix local.hex
RUN mix deps.get
RUN mix release

FROM alpine:latest
COPY --from=build /src/_build/prod/rel/sharity /src
WORKDIR /src
RUN apk add --no-cache libncursesw libstdc++ libgcc libsodium

EXPOSE 80/tcp
ENTRYPOINT ["./bin/sharity", "start"]
